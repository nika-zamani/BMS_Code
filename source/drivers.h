#ifndef DRIVERS_H_
#define DRIVERS_H_

#include "spi.h"
#include "gpio.h"
#include "can.h"

uint8_t spiinit();
uint8_t caninit();
void cantransmit();

void can_uptime_tx();

void bmsOkOut();
void bmsNotOkOut();

#endif
