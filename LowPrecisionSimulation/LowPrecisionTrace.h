#ifndef LOW_PRECISION_TRACE_H
#define LOW_PRECISION_TRACE_H

#include <cstdint>
#include "adele/axpike_storage.h"

class processor_t;

void traceLowPrecisionConversionFP32(
        const char* format,
        processor_t* p,
        source_t* source,
        uint32_t before,
        uint32_t after
);

void traceLowPrecisionConversionFP64(
        const char* format,
        processor_t* p,
        source_t* source,
        uint64_t before,
        uint64_t after
);

#endif // LOW_PRECISION_TRACE_H
