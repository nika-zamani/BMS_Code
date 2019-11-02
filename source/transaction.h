#ifndef TRANSACTION_H
#define TRANSACTION_H

/* Standard includes */
#include "string.h"
#include <vector>
#include <array>

/* FreeRTOS includes */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* Hardware specific includes */

#include "MKE18F16.h"
#include "clock_config.h"
#include "pin_mux.h"

#include "bmsCommand.h"
#include "transaction.h"

#include "gpio.h"
#include "spi.h"

#include <future>

#define SLAVE_COUNT 10

#define t_SLEEP 10000 // find this
#define t_IDLE 1000 // find this

#define ltccsport BSP::gpio::PortA
#define ltccspin 2

using namespace BSP;

void transactionInit();
void transaction( void *pvParameters );

int sendCommandAsync( int com, int length, int num, uint8_t **data, int ticksToWait, uint8_t* result);
uint8_t* sendCommand( int com, int length, int num, uint8_t **data, int ticksToWait );

#endif