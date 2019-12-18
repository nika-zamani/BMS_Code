#ifndef TRANSACTION_H
#define TRANSACTION_H

#include "common.h"

#include "bmsCommand.h"
#include "transaction.h"

#include "gpio.h"
#include "spi.h"

// timeouts defined in us
#define t_SLEEP 100 // TODO: find this
#define t_IDLE 100 // TODO: find this

using namespace BSP;

// GPIO static init
// SPI static init
void transactionInit();


void transaction( void *pvParameters );

// returns 0 on success
uint8_t pushCommand(uint8_t comn, int num, uint8_t *data, 
        uint8_t arg1 = 0, uint8_t arg2 = 0, uint8_t arg3 = 0, uint8_t arg4 = 0);

int sendCommandAsync( int com, int length, int num, uint8_t *data, 
        int ticksToWait, uint8_t** result);


#endif
