#ifndef TYPE_CONVERTION_SOFTFLOAT_H
#define TYPE_CONVERTION_SOFTFLOAT_H

#include <cstdint>

#pragma once

#ifdef INLINE
#  undef INLINE
#endif

#include "softfloat/softfloat.h"

uint32_t typeSimulationSoftFloatFP16(uint32_t value);
uint64_t typeSimulationSoftFloatFP16(uint64_t value);

#endif // TYPE_CONVERTION_SOFTFLOAT_H
