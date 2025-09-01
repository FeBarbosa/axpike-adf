// ex: https://github.com/VArchC/MIPS32r2/blob/sbac2018/mips_varchc_models.cpp

DM ReadLowPrecisionBF16(processor_t* p, source_t* source, void* data) {
    if(source->type == source_t::REGBANK && source->name == "FPR"){// && source->width == 32){
        // printf("source->width: %d\n", source->width);
        // parameters to define the type being simulated
        uint32_t mantissa_size = 7;
        uint32_t* fpreg = (uint32_t*)(data);
        uint32_t fpValue = *fpreg;
        uint32_t exponent = (fpValue >> 23) & 0xFF;
        // float* fpRealValue = (float*)&fpValue;
        // printf("FP32: %f\n", *fpRealValue);
        // if exponent is equal 255, propagates FP32 NaN or Infinity
        if(exponent < 255){
            fpValue &= ~((1u << (23 - mantissa_size)) - 1);
        }
        printf("fpValue: 0x%08x\n\n", fpValue);
        *fpreg = fpValue;
    }
}
