// extern "C" {
// #include "adele/adf/LowPrecisionSimulation/typeConvertion.c"
// }

DM ReadLowPrecisionE5M2(processor_t* p, source_t* source, void* data) {
    if(source->type == source_t::REGBANK && source->name == "FPR"){// && source->width == 32){
        uint32_t* fpreg = (uint32_t*)(data);
        *fpreg = typeSimulationFF(5, 2, *fpreg);

        // float* fpRealValue = (float*)&fpValue;
        // printf("FP32: %f\n", *fpRealValue);
        // *fpreg = typeSimulation(
        //         fpValue,
        //         mantissa_size,
        //         exponent_min,
        //         exponent_max,
        //         mantissa_max
        // );
    }
}
