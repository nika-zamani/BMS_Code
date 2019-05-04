/* bmsutil
 * functions for bms actions which abstract out calls to ltcutility functions
 */

#ifndef BMSUTIL_H_
#define BMSUTIL_H_

#include "defines.h"
#include "actions.h"
#include "ltcutility.h"

// wakeup slaves: 400us cs down, 100us cs up
uint8_t wakeup(void);

// read cells and gpio1,2
uint8_t readall(void);

// !!!
void panic(void);

//
void voltCheck(void);

void stepMux(void);
void muxOn(uint8_t mux, uint8_t pin);
void muxOff(uint8_t mux);

// get temperature from gpio buffer
void getTemp(void);
// check temp limits
uint8_t tempCheck(uint8_t slave, uint8_t id);

void setup();

#endif
