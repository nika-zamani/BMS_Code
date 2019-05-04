#ifndef DRIVERS_H_
#define DRIVERS_H_

#include "spi.h"
#include "gpio.h"

uint8_t spiinit(void);

void bmsOkOut(void);
void bmsNotOkOut(void);

#endif
