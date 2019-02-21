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
    WRCFGA,
    WRCFGB,
    RDCFGA,
    RDCFGB,
    RDCVA,
    RDCVB,
    RDCVC,
    RDCVD,
    RDCVE,
    RDCVF,
    RDAUXA,
    RDAUXB,
    RDAUXC,
    RDAUXD,
    RDSTATA,
    RDSTATB,
    WRSCTRL,
    WRPWM,
    WRPSB,
    RDSCTRL,
    RDPWM,
    RDPSB,
    STSCTRL,
    CLRSCTRL,
    ADCV,
    ADOW,
    CVST,
    ADOL,
    ADAX,
    ADAXD,
    AXST,
    ADSTAT,
    ADSTATD,
    STATST,
    ADCVAX,
    ADCVSC,
    CLRCELL,
    CLRAUX,
    CLRSTAT,
    PLADC,
    DIAGN,
    WRCOMM,
    RDCOMM,
    STCOMM 
};

uint8_t init();

void transmit();

void transmit(uint8_t* data);

void tick();

void wait();

void adcvsc(uint8_t MD= 0b10, uint8_t DCP = 0b1);
void rdcva(void);

void pec15_calc(uint8_t length, uint8_t* data, uint8_t* pec);

}

#endif
