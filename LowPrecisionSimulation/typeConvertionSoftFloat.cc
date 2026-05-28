#include "adele/adf/LowPrecisionSimulation/typeConvertionSoftFloat.h"

uint32_t typeSimulationSoftFloatFP16(uint32_t value)
{
    float32_t in{value};
    float16_t half = f32_to_f16(in);
    float32_t out = f16_to_f32(half);
    return out.v;
}
