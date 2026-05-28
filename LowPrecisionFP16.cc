#include "adele/adf/LowPrecisionSimulation/typeConvertionSoftFloat.h"

DM ReadLowPrecisionFP16(processor_t* p, source_t* source, void* data) {
    if(source->type == source_t::REGBANK && source->name == "FPR"){// && source->width == 32){
        uint32_t* fpreg = (uint32_t*)(data);
        *fpreg = typeSimulationSoftFloatFP16(*fpreg);
    }
}
