/* FreeRTOS includes */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include <string.h>

#define SLAVE_COUNT 1
#define CELL_COUNT 18

#define tick_ms(x) (x * 10)
#define tick_us(x) (x / 100)
