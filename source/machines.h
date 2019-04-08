#ifndef MACHINES_H_
#define MACHINES_H_

#include <stdint.h>
#include <cstddef>
#include "defines.h"

enum slavestates_t {
    off,
    asleep,
    awake
};

enum masterstates_t {
    normalOk,
    normalError,
    charge,
    discharge
};


uint32_t ledstates[][5] = {
    { ms(500), ms(500), 0},
    { ms(200), ms(200), ms(200), ms(800) }
};

template<typename T>
class machine {
private:
    uint32_t timer;
    uint32_t resetval;
    uint8_t on;
public:
    machine(T s, void(*d)(void), void(*dt)(machine*) = NULL);
    T state;
    void (*drive)(void);
    void (*drivethis)(machine* m);
    void run(void);

    void setTimer(uint32_t x){
        resetval=x;
    }
    void tick(void){
        if(on&&timer) timer--;
    }
    void stop(void){
        on = 0;
    }
    void start(void){
        timer=resetval;
        run();
        on = 1;
    }
};

#endif
