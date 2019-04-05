#include "bmsutil.h"
#include "cache.h"

extern cache_t cache;

// todo: insert in place
// todo: don't let actions happen until inserting is done
uint8_t wakeup(void){
    if(actqcheck(4*slaves)) return 1;

    for(uint8_t i = 0; i < slaves; i++){
        actcsdown();
        actwait(us(400));
        actcsup();
        actwait(us(100));
/*
        // inserted in reverse order 
        action_t a;
        a.a = wait;
        a.wait = us(100);
        actinsert(a);
        a.a = csup;
        actinsert(a);
        a.a = wait;
        a.wait = us(400);
        actinsert(a);
        a.a = csdown;
        actinsert(a);
*/
    }

    return 0;
}

uint8_t readall(void){
    
    bms::adcvax();
    bms::rdcva();
    bms::rdcvb();
    bms::rdcvc();
    bms::rdcvd();
    bms::rdauxa();
    actexecute(voltCheck);

    return 0;
}

void panic(void){
    // do all the panicky things
}

void voltCheck(void){

    uint32_t vmax = 0;
    uint32_t vmin = ~0;
    uint32_t vtot = 0;
    uint32_t vavg = 0;

    uint16_t i;
    uint16_t n = slaves * cells;

    for(i = 0; i < n; i++){

        if(cache.volts[i] > vmax) vmax = cache.volts[i];
        if(cache.volts[i] < vmin) vmin = cache.volts[i];
        vtot += cache.volts[i];

    }

    vavg = vtot/n;

    cache.voltageMax = vmax;
    cache.voltageMin = vmin;
    cache.voltageTotal = vtot;
    cache.voltageMean = vavg;

    if(vmax > voltageLimitUpper) {
        cache.voltError();
        panic();
    }
    if(vmin < voltageLimitLower) {
        cache.voltError();
        panic();
    }

}
