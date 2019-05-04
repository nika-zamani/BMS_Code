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
    link,
    startup,
    temps,
    normalOk,
    normalError,
    charge,
    discharge,
    hold
};


uint32_t ledstates[][7] = {
    { ms(1200), ms(1200), 0, 0, 0},               // 0: slow blink
    { ms(200), ms(800), 0 },                     // 1: one blink
    { ms(100), ms(100), ms(100), ms(500), 0},    // 2: two blink
    { ms(200), ms(200), ms(200), ms(200), ms(200), ms(800), 0}, // 3: three blink
    { ms(200), ms(200), 0 },                     // 4: panicky blink
    { ms(1000), 0 }                             // 5: just on
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
