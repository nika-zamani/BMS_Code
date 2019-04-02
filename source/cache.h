#include <stdint.h>
#include "defines.h"

/* cache_t */
// defines a structure for holding persistent data:
// - program flags 
// - info received from slaves
// - calculated values

typedef struct cache_t {

    // flags
    uint8_t allok : 1;
    uint8_t voltok : 1;
    uint8_t tempok : 1;
    // Call to indicate an error has occurred 
    inline void voltError() { allok = 0; voltok = 0; }
    inline void tempError() { allok = 0; tempok = 0; }

    // gpio analog values: 5 per slave, used for temps etc
    uint16_t gpio[5 * slaves];

    // voltages: #cells per slave
    uint16_t volts[cells * slaves];
    uint16_t adcRxMask[slaves] = {0}; // record of received voltages
    // calculated voltage values
    uint16_t voltageMax;
    uint16_t voltageMin;
    uint16_t voltageMean;
    
    // temperatures (calculated from gpio analog signal
    uint16_t temps[thermistors * slaves];
    uint16_t tempMax;
    uint16_t tempMin;



} cache_t;

