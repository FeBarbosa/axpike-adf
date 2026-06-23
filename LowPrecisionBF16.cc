#include "adele/adf/LowPrecisionSimulation/typeConvertion.h"

DM ReadLowPrecisionBF16(processor_t* p, source_t* source, void* data) {
    if(source->type == source_t::REGBANK && source->name == "FPR"){
        uint32_t* fpreg = (uint32_t*)(data);
        *fpreg = typeSimulationFF(8, 7, *fpreg);
    }
}

DM ReadLowPrecisionBF16FP64(processor_t* p, source_t* source, void* data) {
    if(source->type == source_t::REGBANK && source->name == "FPR"){
        uint64_t* fpreg = (uint64_t*)(data);
        *fpreg = typeSimulationFF64(8, 7, *fpreg);
    }
}
