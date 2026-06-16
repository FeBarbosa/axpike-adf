# AxPIKE Low-Precision Type Simulation

This directory contains AxPIKE approximation models written as ADF
descriptions and C/C++ model code. The low-precision floating-point
approximations are implemented by the files with the `LowPrecision` prefix:

- `LowPrecisionFP16.adf` and `LowPrecisionFP16.cc`
- `LowPrecisionBF16.adf` and `LowPrecisionBF16.cc`
- `LowPrecisionE5M2.adf` and `LowPrecisionE5M2.cc`
- `LowPrecisionE4M3.adf` and `LowPrecisionE4M3.cc`

The current active ADF wiring simulates low-precision floating-point behavior
by quantizing FP32 and FP64 register values. FP16 uses SoftFloat round-trips
(`FP32 -> FP16 -> FP32` and `FP64 -> FP16 -> FP64`), while BF16, E5M2, and
E4M3 use `flexfloat`.
The register file does not store packed FP16, BF16, E5M2, or E4M3 values.
Instead, it stores normal FP32 or FP64 values whose numeric values have been
converted through the selected lower-precision format and then converted back
to the instruction's architectural precision.

## Format Mapping

Each model calls one of the conversion helpers below:

| Approximation | FP32 call | FP64 call | Meaning |
| --- | --- | --- | --- |
| FP16 | `typeSimulationSoftFloatFP16(uint32_t)` | `typeSimulationSoftFloatFP16(uint64_t)` | SoftFloat round-trip through FP16 |
| BF16 | `typeSimulationFF(8, 7, value)` | `typeSimulationFF64(8, 7, value)` | 8 exponent bits, 7 mantissa bits |
| E5M2 | `typeSimulationFF(5, 2, value)` | `typeSimulationFF64(5, 2, value)` | 5 exponent bits, 2 mantissa bits |
| E4M3 | `typeSimulationFF(4, 3, value)` | `typeSimulationFF64(4, 3, value)` | 4 exponent bits, 3 mantissa bits |

For example, `LowPrecisionE5M2.cc` applies:

```cpp
uint32_t* fpreg = (uint32_t*)(data);
*fpreg = typeSimulationFF(5, 2, *fpreg);
```

## Conversion Path

The FP16 SoftFloat FP32 conversion helper is implemented in
`LowPrecisionSimulation/typeConvertionSoftFloat.cc`:

```cpp
uint32_t typeSimulationSoftFloatFP16(uint32_t value)
{
    float32_t in{value};
    float16_t half = f32_to_f16(in);
    float32_t out = f16_to_f32(half);
    return out.v;
}
```

The steps are:

1. Take the 32-bit FPR value as raw bits.
2. Reinterpret those bits as a SoftFloat `float32_t`.
3. Convert the value to `float16_t`.
4. Convert the `float16_t` value back to `float32_t`.
5. Return the resulting FP32 bit pattern.

This means the register still contains FP32 bits, but the represented numeric
value has been rounded and range-limited according to the selected low-precision
format.

The FP64 overload follows the same idea, but preserves a 64-bit architectural
result:

```cpp
uint64_t typeSimulationSoftFloatFP16(uint64_t value)
{
    float64_t in{value};
    float16_t half = f64_to_f16(in);
    float64_t out = f16_to_f64(half);
    return out.v;
}
```

## Where AxPIKE Applies the Conversion

The `.adf` files define which instructions receive the low-precision data model.
Each low-precision approximation is initially disabled:

```adf
approximation LOWPRECISIONFP16 {
    initial = off;
    ...
}
```

The approximation becomes active only when selected through AxPIKE's activation
mechanism.

The ADF files divide FP32 instructions into three groups:

- `fp32_readwrite`: instructions that read and write FPR values, such as
  `fadd_s`, `fsub_s`, `fmul_s`, `fdiv_s`, `fsqrt_s`, `fmadd_s`, `flw`, and
  `fsw`.
- `fp32_read`: instructions that only consume FP32 values from FPRs, such as
  `fcvt_w_s`, `fcvt_wu_s`, `fmv_x_w`, `feq_s`, `flt_s`, `fle_s`,
  `fclass_s`, `fcvt_l_s`, and `fcvt_lu_s`.
- `fp32_write`: instructions that produce FP32 values into FPRs, such as
  `fcvt_s_w`, `fcvt_s_wu`, `fmv_w_x`, `fcvt_s_l`, and `fcvt_s_lu`.

For `fp32_readwrite`, AxPIKE applies the conversion on both FPR reads and FPR
writes:

```adf
group fp32_readwrite {
    regbank_read = ReadLowPrecisionFP16();
    regbank_write = ReadLowPrecisionFP16();
}
```

For `fp32_read`, conversion is applied only when reading from the FPR. For
`fp32_write`, conversion is applied only before writing the result into the FPR.

FP64 instructions are handled by analogous groups:

- `fp64_readwrite`: `fld`, `fsd`, `fmadd_d`, `fmsub_d`, `fnmsub_d`,
  `fnmadd_d`, `fadd_d`, `fsub_d`, `fmul_d`, `fdiv_d`, `fsqrt_d`,
  `fsgnj_d`, `fsgnjn_d`, `fsgnjx_d`, `fmin_d`, and `fmax_d`.
- `fp64_read`: FP64 consumers such as `fcvt_w_d`, `fcvt_wu_d`, `fmv_x_d`,
  `fmvh_x_d`, `feq_d`, `flt_d`, `fle_d`, `fclass_d`, `fcvt_l_d`, and
  `fcvt_lu_d`.
- `fp64_write`: FP64 producers such as `fcvt_d_w`, `fcvt_d_wu`, `fmv_d_x`,
  `fmvp_d_x`, `fcvt_d_l`, and `fcvt_d_lu`.
- `fp32_to_fp64`: `fcvt_d_s`, which reads an FP32 value and writes an FP64
  value.
- `fp64_to_fp32`: `fcvt_s_d`, which reads an FP64 value and writes an FP32
  value.

The FP32 and FP64 hooks are intentionally separate. The FP32 hook treats the FPR
payload as a 32-bit raw value; the FP64 hook treats it as a 64-bit raw value.
This avoids truncating double-precision operands when a `_d` instruction is
approximated.

## Execution Semantics

The arithmetic instruction itself is still executed by Spike's normal
single-precision or double-precision implementation. The low-precision behavior
comes from quantizing operands and results at the register interface.

For an active `LOWPRECISIONE5M2` approximation, an `fadd_s` behaves
conceptually like:

```text
a_q = quantize_to_E5M2(a)
b_q = quantize_to_E5M2(b)
r   = normal Spike FP32 fadd_s(a_q, b_q)
r_q = quantize_to_E5M2(r)
FPR[rd] = r_q stored as FP32 bits
```

The same pattern is used for BF16, E5M2, and E4M3, changing only the
`flexfloat<E, M>` format used for quantization.

For an active FP64 instruction such as `fadd_d`, the same boundary model is used
with FP64 storage:

```text
a_q = quantize_to_E5M2(a)
b_q = quantize_to_E5M2(b)
r   = normal Spike FP64 fadd_d(a_q, b_q)
r_q = quantize_to_E5M2(r)
FPR[rd] = r_q stored as FP64 bits
```

## Important Limitations

This is a practical approximation model, not a bit-accurate implementation of
a dedicated low-precision floating-point execution pipeline.

Important consequences:

- Values are not stored in packed FP16, BF16, or FP8 encodings.
- The FPR stores FP32 or FP64 bit patterns after low-precision conversion.
- Spike still performs the arithmetic operation using its normal FP32 or FP64
  logic.
- Low precision is modeled at FPR read/write boundaries.
- Rounding, overflow, underflow, NaN, and infinity behavior are delegated to
  SoftFloat for FP16 and to `flexfloat` for BF16, E5M2, and E4M3.

## Opt-In FP16/E5M2 Conversion Trace

`LowPrecisionSimulation/LowPrecisionTrace.cc` implements optional conversion
tracing for the FP16 and E5M2 hooks. Tracing is disabled by default and does
not change the simulated numeric behavior. The trace is intended as a debug
facility for low-precision conversion analysis and only emits rows when the
matching AxPIKE approximation is active.

The traced hooks are:

- `ReadLowPrecisionFP16`
- `ReadLowPrecisionFP16FP64`
- `ReadLowPrecisionE5M2`
- `ReadLowPrecisionE5M2FP64`

Enable tracing with environment variables:

```bash
AXPIKE_TRACE_LP_CONVERSIONS=1
AXPIKE_TRACE_LP_LIMIT=1000
AXPIKE_TRACE_LP_FILE=/tmp/axpike_lp_conversions.log
AXPIKE_TRACE_LP_FORMAT=fp16,e5m2
AXPIKE_TRACE_LP_WIDTH=fp32,fp64
```

`AXPIKE_TRACE_LP_FORMAT` accepts `fp16`, `e5m2`, comma-separated values, or
`all`. `AXPIKE_TRACE_LP_WIDTH` accepts `fp32`, `fp64`, comma-separated values,
or `all`. If no file is provided, the trace is written to `stderr`. The hard
limit defaults to 1000 lines.

The TSV columns are:

```text
seq	format	width	op	pc	insn_id	raw_insn	before_bits	after_bits	before_value	after_value	before_class	after_class	abs_error	rel_error
```

Column meanings:

- `format`: `FP16` or `E5M2`.
- `width`: architectural hook width, `FP32` or `FP64`.
- `op`: `regbank_read` or `regbank_write`.
- `pc`: current instruction PC from `processor_t::ax_control`.
- `insn_id`: generated AxPIKE instruction id.
- `raw_insn`: raw RISC-V instruction bits.
- `before_bits` and `after_bits`: raw architectural FP32 or FP64 bit patterns.
- `before_value` and `after_value`: decimal interpretation of those raw bits.
- `before_class` and `after_class`: `finite`, `inf`, or `nan`.
- `abs_error` and `rel_error`: only meaningful when both values are finite.

Rebuild the AxPIKE wrapper after changing the trace code:

```bash
make -C build axpike_wrappers.o -j2
```

Example FP16 expf probe from the LeNet repository:

```bash
cd /home/felipe/Research/repos/lenet-riscv-cpp-inference
AXPIKE_TRACE_LP_CONVERSIONS=1 AXPIKE_TRACE_LP_LIMIT=2000 AXPIKE_TRACE_LP_FILE=/tmp/fp16_lp.log AXPIKE_TRACE_LP_FORMAT=fp16 AXPIKE_TRACE_LP_WIDTH=all axpike --adele-activate=LOWPRECISIONFP16 pk app expf-probe expf-probe 10 fp16
```

Example E5M2 expf probe:

```bash
cd /home/felipe/Research/repos/lenet-riscv-cpp-inference
AXPIKE_TRACE_LP_CONVERSIONS=1 AXPIKE_TRACE_LP_LIMIT=2000 AXPIKE_TRACE_LP_FILE=/tmp/e5m2_lp.log AXPIKE_TRACE_LP_FORMAT=e5m2 AXPIKE_TRACE_LP_WIDTH=all axpike --adele-activate=LOWPRECISIONE5M2 pk app expf-probe expf-probe 10 e5m2
```

If the target program activates approximations internally through AxPIKE CSRs or
HTIF commands, the explicit `--adele-activate=...` option is not required. If no
trace file is produced, first confirm that the requested approximation is active
before the floating-point instructions under investigation execute.

When analyzing the log, compare FP32 and FP64 rows around the same PC or
instruction id. For the `expf()` investigation, rows with `width=FP64` show
whether `fcvt.d.s`, double-precision arithmetic, or `fcvt.s.d` paths are being
quantized by FP16 or E5M2. Large `abs_error` or `rel_error` rows identify
conversions where the selected low-precision format changes the value most.

The trace records the raw opcode and generated instruction id. It does not
currently emit a disassembled mnemonic because the hook path does not expose a
mnemonic string directly without adding a larger lookup dependency.
