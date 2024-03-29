/* FreeRTOS includes */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include <string.h>

#define SLAVE_COUNT 5
#define CELL_COUNT 16

#define tick_ms(x) (x * 10)
#define tick_us(x) (x / 100)
