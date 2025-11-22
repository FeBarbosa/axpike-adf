#include "typeConvertion.h"

uint32_t typeSimulation(
    uint32_t fpValue, // In Hexadecimal
    uint32_t mantissa_size,
    uint32_t exponent_min,
    uint32_t exponent_max,
    uint32_t mantissa_max
){
    uint32_t exponent = (fpValue >> 23) & 0xFF; // extrai expoente
    float* fpRealValue = (float*)&fpValue;
    // force exponent of fp32 to max value if greather than max(E) of simulated type
    // Exponent overflow -> force max_exponent
    if (exponent > exponent_max && exponent < 255){ // if exponent is equal 255, propagates FP32 NaN or Infinity
        // printf("AQUI 01\n");                                                    
        exponent = exponent_max;
        fpValue &= ~(static_cast<uint32_t>(0xFF) << 23);
        fpValue |= (exponent << 23);
        // mantain only the <mantissa_size> MSBs of mantissa
        fpValue &= ~((1u << (23 - mantissa_size)) - 1);
    }// Exponent unverflow -> force min_exponent
    else if(exponent < exponent_min && exponent > 0){
        // printf("AQUI 02\n");                                                    
        exponent = exponent_min;
        fpValue &= ~(static_cast<uint32_t>(0xFF) << 23);
        fpValue |= (exponent << 23);
        // mantain only the <mantissa_size> MSBs of mantissa
        fpValue &= ~((1u << (23 - mantissa_size)) - 1);
    }// mantain only the <mantissa_size> MSBs of mantissa
    else if((exponent >= exponent_min && exponent <= exponent_max) || exponent == 0){
        fpValue &= ~((1u << (23 - mantissa_size)) - 1);
    }
    if(mantissa_max != 0){ // Case for E4M3
        // if exponent max, then mantissa need to be == 6
        // OBS: E4M3 does not implement Infinity values
        // and uses only one NaN representation, with all bits 1,
        // so this value is reserved
        if(exponent == exponent_max){ // mantissa condition for exponent_max
            uint32_t mantissa_temp = (fpValue >> (23 - mantissa_size)) & 0x7;
            if(mantissa_temp == mantissa_max+1){
                mantissa_temp == 6;
                fpValue &= ~(0x7 << (23 - mantissa_size));
                fpValue |= (0x6 << (23 - mantissa_size));
            }
        }
    }
    return fpValue;
}

template<uint32_t E, uint32_t M>
uint32_t typeSimulationFF_impl(uint32_t value)
{
        float* fpreg = (float*)(&value);
        flexfloat<E, M> fpFlex = *fpreg;

        *fpreg = (float)fpFlex;
        return value;
}

/* Supported Types
    FP16 -> 5, 10
    BF16 -> 8, 7
    E5M2 -> 5, 2
    E4M3 -> 4, 3
*/
uint32_t typeSimulationFF(
        uint32_t exponent_size,
        uint32_t mantissa_size,
        uint32_t value
){
    switch(exponent_size){
        case 8:
            switch(mantissa_size){
                case 7:
                    // BF16
                    return typeSimulationFF_impl<8, 7>(value);
            }
            break;
        case 5:
            switch(mantissa_size){
                case 10:
                    // FP16
                    return typeSimulationFF_impl<5, 10>(value);
                case 2:
                    // E5M2
                    return typeSimulationFF_impl<5, 2>(value);
            }
            break;
        case 4:
            switch(mantissa_size){
                case 3:
                    // E4M3
                    return typeSimulationFF_impl<4, 3>(value);
            }
            break;
        default:
            throw std::runtime_error("Format not supported");
    }
    throw std::runtime_error("Format not supported");
}

