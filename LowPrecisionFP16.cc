// extern "C" {
// #include "adele/adf/LowPrecisionSimulation/typeConvertion.h"
// }

DM ReadLowPrecisionFP16(processor_t* p, source_t* source, void* data) {
    if(source->type == source_t::REGBANK && source->name == "FPR"){// && source->width == 32){
        // printf("source->width: %d\n", source->width);
        // parameters to define the type being simulated
        // uint32_t mantissa_size = 10;
        // uint32_t exponent_max = 142;
        // uint32_t exponent_min = 113;
        // uint32_t mantissa_max = 0;

        // uint32_t* fpreg = (uint32_t*)(data);
        // // *fpreg = (uint32_t)((*fpreg) | 0x80000000);
        // uint32_t fpValue = *fpreg;
        // uint32_t exponent = (fpValue >> 23) & 0xFF;
        // // float* fpRealValue = (float*)&fpValue;
        // // printf("FP32: %f\n", *fpRealValue);
        // *fpreg = typeSimulation(
        //         fpValue,
        //         mantissa_size,
        //         exponent_min,
        //         exponent_max,
        //         mantissa_max
        // );
        uint32_t* fpreg = (uint32_t*)(data);
        *fpreg = typeSimulationFF(5, 10, *fpreg);

        // *fpreg = fpValue;
    }
}
