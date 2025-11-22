#ifndef TYPE_CONVERTION_H
#define TYPE_CONVERTION_H

#include <cstdint>

#pragma once
/* undefine any nasty INLINE macro defined by other headers (SoftFloat) */
#ifdef INLINE
#  undef INLINE
#endif

#include <flexfloat.hpp>

uint32_t typeSimulation(
    uint32_t fpValue, // In Hexadecimal
    uint32_t mantissa_size,
    uint32_t exponent_min,
    uint32_t exponent_max,
    uint32_t mantissa_max = 0
);

uint32_t typeSimulationFF(
        uint32_t exponent_size,
        uint32_t mantissa_size,
        uint32_t value
);

#endif // TYPE_CONVERTION_H
