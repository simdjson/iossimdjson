

#include <random>
#include <cassert>
#include <cinttypes>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstring>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <vector>
#include <algorithm>
#include <mach/mach_time.h>
#include "datasource.h"



// tries to estimate the frequency, returns 0 on failure
double measure_frequency() {
    mach_timebase_info_data_t info;
    mach_timebase_info(&info);
    const size_t test_duration_in_cycles =
    65536;// 1048576;
    // travis feels strongly about the measure-twice-and-subtract trick.
    auto begin1 = mach_absolute_time();
    size_t cycles = 2 * test_duration_in_cycles;

     __asm volatile(
                   ".align 4\n Lcyclemeasure1:\nsubs %[counter],%[counter],#1\nbne Lcyclemeasure1\n "
                   : /* read/write reg */ [counter] "+r"(cycles));
    auto end1 = mach_absolute_time();
    double nanoseconds1 =
    (double) (end1 - begin1) * (double)info.numer / (double)info.denom;

    auto begin2 = mach_absolute_time();
    cycles = test_duration_in_cycles;
    // I think that this will have a 2-cycle latency on ARM?
    __asm volatile(
                  ".align 4\n Lcyclemeasure2:\nsubs %[counter],%[counter],#1\nbne Lcyclemeasure2\n "
                   : /* read/write reg */ [counter] "+r"(cycles));
    auto end2 = mach_absolute_time();
    double nanoseconds2 =
    (double) (end2 - begin2) * (double)info.numer / (double)info.denom;
    double nanoseconds = (nanoseconds1 - nanoseconds2);
    if ((fabs(nanoseconds - nanoseconds1 / 2) > 0.05 * nanoseconds) or
        (fabs(nanoseconds - nanoseconds2) > 0.05 * nanoseconds)) {
        return 0;
    }
    double frequency = double(test_duration_in_cycles) / nanoseconds;
    return frequency;
}


char * print_freq(char * result, double * estimated_freq) {
    size_t attempt = 1000;
    std::vector<double> freqs;
    for (int i = 0; i < attempt; i++) {
        double freq =measure_frequency();
        if(freq > 0) freqs.push_back(freq);
    }
    if(freqs.size() == 0) {
        result+= sprintf(result, "Could not collect a frequency measure\n");
        return result;
    }
    std::sort(freqs.begin(),freqs.end());
    * estimated_freq = freqs[freqs.size() / 2];
    result+= sprintf(result, "Processor: %f GHz \n", * estimated_freq);
    return result;
}

#include "simdjson.h"

extern "C" {
    
    int openable(const char*filename) {
        FILE *f = fopen(filename,"r");
        if (!f)
            return 0; /* openable */
        fclose(f);
        return 1; /* not openable */
    }
    

    char * measure(size_t length, char * result, const char * filename) {
        double frequency;

        const char * justname = strrchr(filename, '/');
        result+= sprintf(result, " %s \n", justname+1);
        
        
        simdjson::padded_string p = simdjson::get_corpus(filename);
        simdjson::ParsedJson pj;
        bool isok = pj.allocateCapacity(p.size());
        if(!isok) {
            result+= sprintf(result, "could not allocate memory  \n");
            return result;
        }
        for(int k = 0;  k < 3; k++) {
            printf("trial: %d \n", k);
        
          const uint64_t startTime1 = mach_absolute_time();
          simdjson::ParsedJson pj = build_parsed_json(p);
        
          const uint64_t endTime1 = mach_absolute_time();
        
          mach_timebase_info_data_t info;
          mach_timebase_info(&info);
          const double elapsedNS = (double) (endTime1 - startTime1) * (double)info.numer / (double)info.denom;
           result+= sprintf(result, "speed of %.3f GB/s \n", p.size() / (1024.0*1024.0*1024.0) * (1000.0 * 1000.0 * 1000.0) / elapsedNS );
        

        }
        result = print_freq(result, &frequency);
        result+= sprintf(result, "\n");
        
        return result;
    }


}
