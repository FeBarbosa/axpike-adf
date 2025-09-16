// ex: https://github.com/VArchC/MIPS32r2/blob/sbac2018/mips_varchc_models.cpp

DM ReadLowPrecisionE5M2(processor_t* p, source_t* source, void* data) {
    if(source->type == source_t::REGBANK && source->name == "FPR"){// && source->width == 32){
        // printf("source->width: %d\n", source->width);
        // parameters to define the type being simulated
        uint32_t mantissa_size = 2;
        uint32_t exponent_max = 142;
        uint32_t exponent_min = 113;

        uint32_t* fpreg = (uint32_t*)(data);
        // *fpreg = (uint32_t)((*fpreg) | 0x80000000);
        uint32_t fpValue = *fpreg;
        uint32_t exponent = (fpValue >> 23) & 0xFF;
        float* fpRealValue = (float*)&fpValue;
        // printf("FP32: %f\n", *fpRealValue);

        // force exponent of fp32 to max value of fp16 if greather than max(E) of fp16
        // Exponent overflow -> force max_exponent
        if (exponent > exponent_max && exponent < 255){ // if exponent is equal 255, propagates FP32 NaN or Infinity
            exponent = exponent_max;
            fpValue &= ~(static_cast<uint32_t>(0xFF) << 23);
            fpValue |= (exponent << 23);
            // mantain only the 10 MSBs of mantissa
            fpValue &= ~((1u << (23 - mantissa_size)) - 1);
        }// Exponent unverflow -> force min_exponent
        else if(exponent < exponent_min && exponent > 0){
            exponent = exponent_min;
            fpValue &= ~(static_cast<uint32_t>(0xFF) << 23);
            fpValue |= (exponent << 23);
            // mantain only the 10 MSBs of mantissa
            fpValue &= ~((1u << (23 - mantissa_size)) - 1);
        }// exponent between accpetable values and denormal
        else if((exponent >= exponent_min && exponent <= exponent_max) || exponent == 0){
            fpValue &= ~((1u << (23 - mantissa_size)) - 1);
        }
        *fpreg = fpValue;
    }
}
