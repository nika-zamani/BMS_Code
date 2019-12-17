#ifndef TRANSACTION_H
#define TRANSACTION_H

#include "common.h"

#include "bmsCommand.h"
#include "transaction.h"

#include "gpio.h"
#include "spi.h"

// timeouts defined in us
#define t_SLEEP 10000 // TODO: find this
#define t_IDLE 100 // TODO: find this

using namespace BSP;

// GPIO static init
// SPI static init
void transactionInit();


void transaction( void *pvParameters );

int sendCommandAsync( int com, int length, int num, uint8_t *data, 
        int ticksToWait, uint8_t** result);

/* To call a command that does not require any special command bits use this command with the name of the command (eg. WRCFGA) for the first argument
 *  @param length:  the length of each peice of data
 *  @param num: the number of peices of data, also the number of  devices in the daisy chain
 *  @param data: data for each device, formated [[dataForDevice[1]], [dataForDevice[2]], [dataForDevice[3]], ... , [dataForDevice[num]]]
 *  @param ticksToWait: the maximum number of tick to wait while calling the command, this time is split evenly between adding the command to the command stack
 *      and waiting for the command to complete and return
 * 
 *  @return the data returned in rx from the command or NULL if the command timed out 
 */
void sendCommand( int com, int length, int num, uint8_t *data, uint8_t *rx, int ticksToWait );

#endif
