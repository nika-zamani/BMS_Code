/* cache_t */
// defines a structure for holding persistent data:
// - program flags 
// - info received from slaves
// - calculated values

#ifndef CACHE_H_
#define CACHE_H_

#include <stdint.h>
#include "defines.h"

typedef struct cache_t {

    // flags
    uint8_t allok : 1;
    uint8_t voltok : 1;
    uint8_t tempok : 1;
    uint8_t commsok : 1;
    // Call to indicate an error has occurred 
    inline void allStatus() { allok = voltok && tempok && commsok; }
    inline void voltError() { voltok = 0; allStatus(); }
    inline void tempError() { tempok = 0; allStatus(); }
    inline void commsError() { commsok = 0, allStatus(); }
    inline void commsGood() { commsok = 1, allStatus(); }

    // gpio analog values: 5 per slave, used for temps etc
    uint16_t gpio[5 * slaves];

    // voltages: #cells per slave
    uint16_t volts[cells * slaves];
    uint16_t adcRxMask[slaves] = {0}; // record of received voltages
    // calculated voltage values
    uint16_t voltageMax;
    uint16_t voltageMin;
    uint16_t voltageMean;
    uint32_t voltageTotal;
    
    // temperatures (calculated from gpio analog signal
    uint16_t temps[thermistors * slaves];
    uint16_t tempMax;
    uint16_t tempMin;

    // chip registers read from 6811
    uint8_t comm[slaves][6];
    uint8_t configA[slaves][6];
    uint8_t statA[slaves][6];

    // mux state
    uint8_t mux;
    uint8_t muxpin;

} cache_t;

#endif
