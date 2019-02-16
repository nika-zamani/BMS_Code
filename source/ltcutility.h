#ifndef LTCUTILITY_H_
#define LTCUTILITY_H_

#include "Scheduler.h"
#include "gpio.h"

namespace bms{

enum bmsstate_t {
    wakedown,
    wakeup,
    transmitting,
    ready, 
    error, 
    idle
};

enum bmscommands_t {
    readConfig,
    writeConfig
};

uint8_t bmsinit();

void bmstransmit();

void bmstick();

void pec15_calc(uint8_t length, uint8_t* data, uint8_t* pec);

}

#endif
