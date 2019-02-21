#define slaves 2

#include <stdint.h>

typedef struct cache_t {

    uint8_t allok : 1;
    uint8_t voltok : 1;
    uint8_t tempok : 1;

    uint16_t volts[11 * slaves];
    uint16_t temps[12 * slaves];

} cache_t;

