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
#define t_IDLE 100 // find this

#define ltccsport BSP::gpio::PortA
#define ltccspin 2
#define ltcbaud 500000U

using namespace BSP;

// GPIO static init
// SPI static init
void transactionInit();


void transaction( void *pvParameters );

int sendCommandAsync( int com, int length, int num, uint8_t *data, int ticksToWait, uint8_t** result);

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

// call ADCV command like any other but with base 10 integer values for MD, DCP, and CH
uint8_t* sendCommandADCV( int md, int dcp, int ch, int length, int num, uint8_t **data, int ticksToWait );

// call ADOW command like any other but with base 10 integer values for MD, PUP, DCP, and CH
uint8_t* sendCommandADOW( int md, int pup, int dcp, int ch, int length, int num, uint8_t **data, int ticksToWait );

// call CVST command like any other but with base 10 integer values for MD, and ST
uint8_t* sendCommandCVST( int md, int st, int length, int num, uint8_t **data, int ticksToWait );

// call ADOL command like any other but with base 10 integer values for MD, and DCP
uint8_t* sendCommandADOL( int md, int dcp, int length, int num, uint8_t **data, int ticksToWait );

// call ADAX command like any other but with base 10 integer values for MD, and CHG
uint8_t* sendCommandADAX( int md, int chg, int length, int num, uint8_t **data, int ticksToWait );

// call ADAXD command like any other but with base 10 integer values for MD, and CHG
uint8_t* sendCommandADAXD( int md, int chg, int length, int num, uint8_t **data, int ticksToWait );

// call AXST command like any other but with base 10 integer values for MD, and ST
uint8_t* sendCommandAXST( int md, int st, int length, int num, uint8_t **data, int ticksToWait );

// call ADSTAT command like any other but with base 10 integer values for MD, and CHST
uint8_t* sendCommandADSTAT( int md, int chst, int length, int num, uint8_t **data, int ticksToWait );

// call ADSTATD command like any other but with base 10 integer values for MD, and CHST
uint8_t* sendCommandADSTATD( int md, int chst, int length, int num, uint8_t **data, int ticksToWait );

// call STATST command like any other but with base 10 integer values for MD, and ST
uint8_t* sendCommandSTATST( int md, int st, int length, int num, uint8_t **data, int ticksToWait );

// call ADCVAX command like any other but with base 10 integer values for MD, and DCP
uint8_t* sendCommandADCVAX( int md, int dcp, int length, int num, uint8_t **data, int ticksToWait );

// call ADCVSC command like any other but with base 10 integer values for MD, and DCP
uint8_t* sendCommandADCVSC( int md, int dcp, int length, int num, uint8_t **data, int ticksToWait );
#endif
