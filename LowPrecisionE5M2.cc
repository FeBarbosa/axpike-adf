#include "adele/adf/LowPrecisionSimulation/typeConvertion.h"
#include "adele/adf/LowPrecisionSimulation/LowPrecisionTrace.h"

DM ReadLowPrecisionE5M2(processor_t* p, source_t* source, void* data) {
    if(source->type == source_t::REGBANK && source->name == "FPR"){
        uint32_t* fpreg = (uint32_t*)(data);
        uint32_t before = *fpreg;
        uint32_t after = typeSimulationFF(5, 2, before);
        traceLowPrecisionConversionFP32("E5M2", p, source, before, after);
        *fpreg = after;
    }
}

DM ReadLowPrecisionE5M2FP64(processor_t* p, source_t* source, void* data) {
    if(source->type == source_t::REGBANK && source->name == "FPR"){
        uint64_t* fpreg = (uint64_t*)(data);
        uint64_t before = *fpreg;
        uint64_t after = typeSimulationFF64(5, 2, before);
        traceLowPrecisionConversionFP64("E5M2", p, source, before, after);
        *fpreg = after;
    }
}
