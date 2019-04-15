#ifndef MACHINES_H_
#define MACHINES_H_

#include <stdint.h>
#include <cstddef>
#include "defines.h"
#include "gpio.h"

enum slavestates_t {
    off,
    asleep,
    awake
};

enum masterstates_t {
    startup,
    temps,
    normalOk,
    normalError,
    charge,
    discharge,
    hold
};


uint32_t ledstates[][5] = {
    { ms(500), ms(500), 0, 0, 0},
    { ms(200), ms(200), ms(200), ms(800), 0}
};

typedef struct leddata {
    uint8_t counter = 0;
    BSP::gpio::GPIO_port port;
    uint8_t pin;
} leddata;

template<typename T>
class machine {
private:
    uint32_t timer;
    uint32_t resetval;
    uint8_t on;
public:
    void* data;
    machine(T s, void(*d)(void), void(*dt)(machine*) = NULL, void* userdata = NULL);
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
