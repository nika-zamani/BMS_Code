/* cache_t */
// defines a structure for holding persistent data:
// - program flags 
// - info received from slaves
// - calculated values

#ifndef CACHE_H_
#define CACHE_H_

#include <stdint.h>
#include "defines.h"

typedef struct timeout_t {

    uint16_t charger;
    uint16_t can;

    void tick(){
        charger++;
        can++;
    }

} timeout_t;

typedef struct cache_t {

    timeout_t timeout;

    // flags
    uint8_t allok : 1;
    uint8_t voltok : 1;
    uint8_t tempok : 1;
    uint8_t commsok : 1;
    uint8_t reset : 1;
    // Call to indicate an error has occurred 
    inline void allStatus() { allok = voltok && tempok && commsok; }
    inline void voltError() { voltok = 0; allStatus(); }
    inline void voltGood() { voltok = 1; allStatus(); }
    inline void tempError() { tempok = 0; allStatus(); }
    inline void tempOk() { tempok = 1; allStatus(); }
    inline void commsError() { commsok = 0, allStatus(); }
    inline void commsGood() { commsok = 1, allStatus(); }

    // connection flags
    uint8_t charger : 1;
    uint8_t vcu : 1;
    uint8_t dash : 1;

    // used for connection to slaves (unimplemented??)
    uint8_t linked;
    uint8_t confidence;
    enum class commsEC_t { OK, DISCONNECT };
    commsEC_t commsEC;
    uint8_t commsED;

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
    enum class voltEC_t { OK, OVERVOLT, UNDERVOLT};
    voltEC_t voltEC;
    uint8_t voltED[3];
    
    // temperatures (calculated from gpio analog signal
    uint16_t temps[thermistors * slaves];
    uint16_t tempMax;
    uint16_t tempMin;
    uint16_t tempArray[slaves]; // binary representation of functioning temps
    uint16_t tempGood[slaves]; // binary representation of acceptable temps
    uint16_t tempBroken; // count of broken temp sensors
    enum class tempEC_t { OK, OVERTEMP, UNDERTEMP, BROKEN };
    tempEC_t tempEC;
    uint8_t tempED;

    // chip registers read from 6811
    uint8_t comm[slaves][6];
    uint8_t configA[slaves][6];
    uint8_t statA[slaves][6];

    // mux state
    uint8_t mux;
    uint8_t muxpin;

} cache_t;

#endif
