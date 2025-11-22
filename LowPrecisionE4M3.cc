DM ReadLowPrecisionE4M3(processor_t* p, source_t* source, void* data) {
    if(source->type == source_t::REGBANK && source->name == "FPR"){// && source->width == 32){
        // printf("source->width: %d\n", source->width);
        // parameters to define the type being simulated
        // uint32_t mantissa_size = 3;
        // uint32_t exponent_max = 135;
        // uint32_t exponent_min = 121;
        // uint32_t mantissa_max = 6;

        // uint32_t* fpreg = (uint32_t*)(data);
        // // *fpreg = (uint32_t)((*fpreg) | 0x80000000);
        // uint32_t fpValue = *fpreg;
        // uint32_t exponent = (fpValue >> 23) & 0xFF;
        // float* fpRealValue = (float*)&fpValue;
        // // printf("FP32: %f\n", *fpRealValue);
        // *fpreg = typeSimulation(
        //         fpValue,
        //         mantissa_size,
        //         exponent_min,
        //         exponent_max,
        //         mantissa_max
        // );
        // *fpreg = fpValue;
        //
        uint32_t* fpreg = (uint32_t*)(data);
        *fpreg = typeSimulationFF(4, 3, *fpreg);
    }
}
