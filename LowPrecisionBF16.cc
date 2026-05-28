#include "adele/adf/LowPrecisionSimulation/typeConvertion.h"

DM ReadLowPrecisionBF16(processor_t* p, source_t* source, void* data) {
    if(source->type == source_t::REGBANK && source->name == "FPR"){// && source->width == 32){
        // printf("source->width: %d\n", source->width);
        // parameters to define the type being simulated
        // uint32_t mantissa_size = 7;
        // uint32_t* fpreg = (uint32_t*)(data);
        // uint64_t* fpreg64 = (uint64_t*)(data);
        // uint32_t fpValue = *fpreg;
        // uint64_t fpValue64 = *fpreg64;
        // uint32_t exponent = (fpValue >> 23) & 0xFF;
        // // float* fpRealValue = (float*)&fpValue;
        // // printf("FP32: %f\n", *fpRealValue);
        // // if exponent is equal 255, propagates FP32 NaN or Infinity
        // if(exponent < 255){
        //     fpValue &= ~((1u << (23 - mantissa_size)) - 1);
        // }
        // // printf("fpValue: 0x%08x\n\n", fpValue);
        // // printf("fpValue: 0x%016x\n\n", fpValue64);
        // *fpreg = fpValue;

        uint32_t* fpreg = (uint32_t*)(data);
        *fpreg = typeSimulationFF(8, 7, *fpreg);
    }
}
