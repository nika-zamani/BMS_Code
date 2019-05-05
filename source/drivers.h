#ifndef DRIVERS_H_
#define DRIVERS_H_

#include "spi.h"
#include "gpio.h"
#include "canlight.h"

uint8_t spiinit();
uint8_t caninit();
void cantransmit();

void bmsOkOut();
void bmsNotOkOut();

#endif
