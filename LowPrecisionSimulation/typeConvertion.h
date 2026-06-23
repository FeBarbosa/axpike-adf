#ifndef TYPE_CONVERTION_H
#define TYPE_CONVERTION_H

#include <cstdint>

#pragma once
/* undefine any nasty INLINE macro defined by other headers */
#ifdef INLINE
#  undef INLINE
#endif

#include <flexfloat.hpp>

uint32_t typeSimulationFF(
        uint32_t exponent_size,
        uint32_t mantissa_size,
        uint32_t value
);

uint64_t typeSimulationFF64(
        uint32_t exponent_size,
        uint32_t mantissa_size,
        uint64_t value
);

#endif // TYPE_CONVERTION_H
