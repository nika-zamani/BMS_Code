/* transaction.*: LTC6811 interaction tool */

/* how to use this tool
 *
 *
 * to start with, call transactionInit() before starting the scheduler. this
 * will create the necessary queues and semaphores.
 *
 * also create the transaction task before starting the scheduler.
 *
 *
 * three basic manners of pushing a command:
 * (this is based on the theory that all outgoing writes are 6 bytes long per
 * slave, and all incoming reads are 6 bytes long per slave. this is because
 * all registers in the LTC6811 are in groups of 6.)
 *
 *  outgoing data:
 *      uint8_t dataout[SLAVE_COUNT][6] = { 0x00, ...
 *      pushCommand(WRCFGA, SLAVE_COUNT, dataout);
 *  in this example, dataout will be formatted and sent out along with the
 *  WRCFGA command.
 *
 *  incoming data:
 *      uint8_t datain[SLAVE_COUNT][6];
 *      pushCommand(RDCFGA, SLAVE_COUNT, datain);
 *  in this example, the received data from RDCFGA will be moved into datain.
 *
 *  clocking a data bus:
 *      uint8_t bytes = 3;
 *      pushCommand(STCOMM, SLAVE_COUNT, &bytes);
 *  this will create 3 bytes worth of clock pulses.
 *
 *  see test/commandSend.cpp for concrete example tasks.
 */

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

// semaphores and queues
void transactionInit();

// the transaction task
void transaction( void *pvParameters );

// see top of file for particular usage guidelines
// returns 0 on success
uint8_t pushCommand(uint8_t comn, int num, uint8_t *data, 
        uint8_t arg1 = 0, uint8_t arg2 = 0, uint8_t arg3 = 0, uint8_t arg4 = 0);

int sendCommandAsync( int com, int length, int num, uint8_t *data, 
        int ticksToWait, uint8_t** result);


#endif
