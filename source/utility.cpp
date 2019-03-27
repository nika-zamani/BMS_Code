#include "defines.h"
#include "cache.h"
#include "utility.h"

extern cache_t cache;

void diagnoseVolts(){

    uint32_t vmax = 0;
    uint32_t vmin = ~0;
    uint32_t vavg = 0;

    uint16_t i;
    uint16_t n = slaves * cells;

    for(i = 0; i < n; i++){

        if(cache.volts[i] > vmax) vmax = cache.volts[i];
        if(cache.volts[i] < vmin) vmin = cache.volts[i];
        vavg += cache.volts[i];

    }

    vavg /= n;

    cache.voltageMax = vmax;
    cache.voltageMin = vmin;
    cache.voltageMean = vavg;

    if(vmax > voltageLimitUpper) cache.voltError();
    if(vmin < voltageLimitLower) cache.voltError();

}

void panic(){

    while(1);

}


