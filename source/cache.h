#include <stdint.h>
#include "defines.h"

typedef struct cache_t {

    uint8_t allok : 1;
    uint8_t voltok : 1;
    uint8_t tempok : 1;

    uint16_t voltageMax;
    uint16_t voltageMin;
    uint16_t voltageMean;

    uint16_t tempMax;
    uint16_t tempMin;

    uint16_t volts[cells * slaves];
    uint16_t temps[thermistors * slaves];

    uint16_t adcRxMask[slaves];

    inline void voltError() { allok = 0; voltok = 0; }

} cache_t;

