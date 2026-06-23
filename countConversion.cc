// extern "C" {
#include "adele/adf/LowPrecisionSimulation/typeConvertion.c"
// }
//

double calc_error(uint32_t realValue, uint32_t producedValue){
    float* testreal = (float*)&realValue;
    float* testProduced = (float*)&producedValue;
    if(realValue != 0){
       return ((double)((*testreal) - (double)(*testProduced))/(double)(*testreal)) * 100; 
    }
    return 0.0;
}

double calc_errorExp(uint32_t realValue, uint32_t producedValue){
    if(realValue != 0){
       return (((double)realValue - (double)producedValue)/(double)realValue) * 100; 
    }
    return 0.0;
}

DM countConversions(processor_t* p, source_t* source, void* data) {
    if(source->type == source_t::REGBANK && source->name == "FPR"){// && source->width == 32){
        uint32_t* fpreg = (uint32_t*)(data);
        uint32_t testFP16, testBF16, testE5M2, testE4M3;
        uint32_t testFP32 = *fpreg;
        double errorFP16, errorBF16, errorE5M2, errorE4M3;

        testFP16 = typeSimulationFF(5, 10, testFP32);
        testBF16 = typeSimulationFF(8, 7, testFP32);
        testE5M2 = typeSimulationFF(5, 2, testFP32);
        testE4M3 = typeSimulationFF(4, 3, testFP32);

        COUNTERS(uint64_t, "FP32")++;

        if(testFP16 == *fpreg){
            COUNTERS(uint64_t, "FP16")++;
        }
        if(testBF16 == *fpreg){
            COUNTERS(uint64_t, "BF16")++;
        }
        if(testE5M2 == *fpreg){
            COUNTERS(uint64_t, "E5M2")++;
        }
        if(testE4M3 == *fpreg){
            COUNTERS(uint64_t, "E4M3")++;
        }

        errorFP16 = calc_error(testFP32, testFP16);
        errorBF16 = calc_error(testFP32, testBF16);
        errorE5M2 = calc_error(testFP32, testE5M2);
        errorE4M3 = calc_error(testFP32, testE4M3);

        // Error 1%
        if(errorFP16 <= 1.0){
            COUNTERS(uint64_t, "FP16-1%")++;
        }
        if(errorBF16 <= 1.0){
            COUNTERS(uint64_t, "BF16-1%")++;
        }
        if(errorE5M2 <= 1.0){
            COUNTERS(uint64_t, "E5M2-1%")++;
        }
        if(errorE4M3 <= 1.0){
            COUNTERS(uint64_t, "E4M3-1%")++;
        }

        // Error 5%
        if(errorFP16 <= 5.0){
            COUNTERS(uint64_t, "FP16-5%")++;
        }
        if(errorBF16 <= 5.0){
            COUNTERS(uint64_t, "BF16-5%")++;
        }
        if(errorE5M2 <= 5.0){
            COUNTERS(uint64_t, "E5M2-5%")++;
        }
        if(errorE4M3 <= 5.0){
            COUNTERS(uint64_t, "E4M3-5%")++;
        }

        // Error 10%
        if(errorFP16 <= 10.0){
            COUNTERS(uint64_t, "FP16-10%")++;
        }
        if(errorBF16 <= 10.0){
            COUNTERS(uint64_t, "BF16-10%")++;
        }
        if(errorE5M2 <= 10.0){
            COUNTERS(uint64_t, "E5M2-10%")++;
        }
        if(errorE4M3 <= 10.0){
            COUNTERS(uint64_t, "E4M3-10%")++;
        }

        testFP32 &= 0xFF800000;
        testFP16 &= 0xFF800000;
        testBF16 &= 0xFF800000;
        testE5M2 &= 0xFF800000;
        testE4M3 &= 0xFF800000;

        // no mantissa test
        if(testFP16 == testFP32){
            COUNTERS(uint64_t, "FP16-exp")++;
        }
        if(testBF16 == testFP32){
            COUNTERS(uint64_t, "BF16-exp")++;
        }
        if(testE5M2 == testFP32){
            COUNTERS(uint64_t, "E5M2-exp")++;
        }
        if(testE4M3 == testFP32){
            COUNTERS(uint64_t, "E4M3-exp")++;
        }

        errorFP16 = calc_errorExp(testFP32, testFP16);
        errorBF16 = calc_errorExp(testFP32, testBF16);
        errorE5M2 = calc_errorExp(testFP32, testE5M2);
        errorE4M3 = calc_errorExp(testFP32, testE4M3);

        // Error 1%
        if(errorFP16 <= 1.0){
            COUNTERS(uint64_t, "FP16-exp-1%")++;
        }
        if(errorBF16 <= 1.0){
            COUNTERS(uint64_t, "BF16-exp-1%")++;
        }
        if(errorE5M2 <= 1.0){
            COUNTERS(uint64_t, "E5M2-exp-1%")++;
        }
        if(errorE4M3 <= 1.0){
            COUNTERS(uint64_t, "E4M3-exp-1%")++;
        }

        // Error 5%
        if(errorFP16 <= 5.0){
            COUNTERS(uint64_t, "FP16-exp-5%")++;
        }
        if(errorBF16 <= 5.0){
            COUNTERS(uint64_t, "BF16-exp-5%")++;
        }
        if(errorE5M2 <= 5.0){
            COUNTERS(uint64_t, "E5M2-exp-5%")++;
        }
        if(errorE4M3 <= 5.0){
            COUNTERS(uint64_t, "E4M3-exp-5%")++;
        }

        // Error 10%
        if(errorFP16 <= 10.0){
            COUNTERS(uint64_t, "FP16-exp-10%")++;
        }
        if(errorBF16 <= 10.0){
            COUNTERS(uint64_t, "BF16-exp-10%")++;
        }
        if(errorE5M2 <= 10.0){
            COUNTERS(uint64_t, "E5M2-exp-10%")++;
        }
        if(errorE4M3 <= 10.0){
            COUNTERS(uint64_t, "E4M3-exp-10%")++;
        }
    }
}
