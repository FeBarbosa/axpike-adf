#include "adele/adf/LowPrecisionSimulation/typeConvertionSoftFloat.h"
#include "adele/adf/LowPrecisionSimulation/LowPrecisionTrace.h"

DM ReadLowPrecisionFP16(processor_t* p, source_t* source, void* data) {
    if(source->type == source_t::REGBANK && source->name == "FPR"){
        uint32_t* fpreg = (uint32_t*)(data);
        uint32_t before = *fpreg;
        uint32_t after = typeSimulationSoftFloatFP16(before);
        traceLowPrecisionConversionFP32("FP16", p, source, before, after);
        *fpreg = after;
    }
}

DM ReadLowPrecisionFP16FP64(processor_t* p, source_t* source, void* data) {
    if(source->type == source_t::REGBANK && source->name == "FPR"){
        uint64_t* fpreg = (uint64_t*)(data);
        uint64_t before = *fpreg;
        uint64_t after = typeSimulationSoftFloatFP16(before);
        traceLowPrecisionConversionFP64("FP16", p, source, before, after);
        *fpreg = after;
    }
}
