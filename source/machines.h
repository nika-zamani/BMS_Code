#ifndef MACHINES_H_
#define MACHINES_H_

#include <stdint.h>

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

template<typename T>
class machine {
private:
    uint32_t timer;
    uint32_t resetval;
    uint8_t on;
public:
    machine(T s, void(*d)(void));
    T state;
    void (*drive)(void);
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
