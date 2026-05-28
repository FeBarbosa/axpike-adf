#include "adele/adf/LowPrecisionSimulation/typeConvertion.h"

DM ReadLowPrecisionE4M3(processor_t* p, source_t* source, void* data) {
    if(source->type == source_t::REGBANK && source->name == "FPR"){// && source->width == 32){
        uint32_t* fpreg = (uint32_t*)(data);
        *fpreg = typeSimulationFF(4, 3, *fpreg);
    }
}
