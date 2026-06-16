#include "typeConvertion.h"
#include <cstring>

template<uint32_t E, uint32_t M>
uint32_t typeSimulationFF_impl(uint32_t value)
{
        float fpreg;
        std::memcpy(&fpreg, &value, sizeof(fpreg));
        flexfloat<E, M> fpFlex = fpreg;

        fpreg = (float)fpFlex;
        std::memcpy(&value, &fpreg, sizeof(value));
        return value;
}

template<uint32_t E, uint32_t M>
uint64_t typeSimulationFF64_impl(uint64_t value)
{
        double fpreg;
        std::memcpy(&fpreg, &value, sizeof(fpreg));
        flexfloat<E, M> fpFlex = fpreg;

        fpreg = (double)fpFlex;
        std::memcpy(&value, &fpreg, sizeof(value));
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

uint64_t typeSimulationFF64(
        uint32_t exponent_size,
        uint32_t mantissa_size,
        uint64_t value
){
    switch(exponent_size){
        case 8:
            switch(mantissa_size){
                case 7:
                    // BF16
                    return typeSimulationFF64_impl<8, 7>(value);
            }
            break;
        case 5:
            switch(mantissa_size){
                case 10:
                    // FP16
                    return typeSimulationFF64_impl<5, 10>(value);
                case 2:
                    // E5M2
                    return typeSimulationFF64_impl<5, 2>(value);
            }
            break;
        case 4:
            switch(mantissa_size){
                case 3:
                    // E4M3
                    return typeSimulationFF64_impl<4, 3>(value);
            }
            break;
        default:
            throw std::runtime_error("Format not supported");
    }
    throw std::runtime_error("Format not supported");
}
