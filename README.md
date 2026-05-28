# AxPIKE Low-Precision Type Simulation

This directory contains AxPIKE approximation models written as ADF
descriptions and C/C++ model code. The low-precision floating-point
approximations are implemented by the files with the `LowPrecision` prefix:

- `LowPrecisionFP16.adf` and `LowPrecisionFP16.cc`
- `LowPrecisionBF16.adf` and `LowPrecisionBF16.cc`
- `LowPrecisionE5M2.adf` and `LowPrecisionE5M2.cc`
- `LowPrecisionE4M3.adf` and `LowPrecisionE4M3.cc`

The current implementation simulates low-precision floating-point behavior by
quantizing FP32 register values. FP16 now uses the SoftFloat round-trip
`FP32 -> FP16 -> FP32`, while BF16, E5M2, and E4M3 still use `flexfloat`.
The register file does not store packed FP16, BF16, E5M2, or E4M3 values.
Instead, it stores a normal 32-bit float whose numeric value has been converted
through the selected lower-precision format and then converted back to FP32.

## Format Mapping

Each model calls one of the conversion helpers below:

| Approximation | Call | Meaning |
| --- | --- | --- |
| FP16 | `typeSimulationSoftFloatFP16(value)` | SoftFloat FP32 -> FP16 -> FP32 |
| BF16 | `typeSimulationFF(8, 7, value)` | 8 exponent bits, 7 mantissa bits |
| E5M2 | `typeSimulationFF(5, 2, value)` | 5 exponent bits, 2 mantissa bits |
| E4M3 | `typeSimulationFF(4, 3, value)` | 4 exponent bits, 3 mantissa bits |

For example, `LowPrecisionE5M2.cc` applies:

```cpp
uint32_t* fpreg = (uint32_t*)(data);
*fpreg = typeSimulationFF(5, 2, *fpreg);
```

## Conversion Path

The FP16 SoftFloat conversion helper is implemented in
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

The ADF files divide instructions into three groups:

- `fp32_readwrite`: instructions that read and write FPR values, such as
  `fadd_s`, `fsub_s`, `fmul_s`, `fdiv_s`, `fsqrt_s`, `fmadd_s`, `flw`, and
  `fsw`.
- `fp32_read`: instructions that only consume FP32 values from FPRs, such as
  `fcvt_w_s`, `fcvt_wu_s`, `fmv_x_w`, `feq_s`, `flt_s`, `fle_s`,
  `fclass_s`, and `fcvt_d_s`.
- `fp32_write`: instructions that produce FP32 values into FPRs, such as
  `fcvt_s_w`, `fcvt_s_wu`, `fmv_w_x`, `fcvt_s_l`, `fcvt_s_lu`, and
  `fcvt_s_d`.

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

## Execution Semantics

The arithmetic instruction itself is still executed by Spike's normal
single-precision implementation. The low-precision behavior comes from
quantizing operands and results at the register interface.

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

## Important Limitations

This is a practical approximation model, not a bit-accurate implementation of
a dedicated low-precision floating-point execution pipeline.

Important consequences:

- Values are not stored in packed FP16, BF16, or FP8 encodings.
- The FPR stores FP32 bit patterns after low-precision conversion.
- Spike still performs the arithmetic operation using its normal FP32 logic.
- Low precision is modeled at FPR read/write boundaries.
- Rounding, overflow, underflow, NaN, and infinity behavior are delegated to
  SoftFloat for FP16 and to `flexfloat` for BF16, E5M2, and E4M3.
