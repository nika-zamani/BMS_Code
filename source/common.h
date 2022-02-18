/* FreeRTOS includes */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include <string.h>

#define SLAVE_COUNT 3
// change this for daisy chain -> 10

#define ltccsport BSP::gpio::PortA
#define ltccspin 2
#define ltcbaud 500000U

#define tick_ms(x) (x*10)
#define tick_us(x) (x/100)
