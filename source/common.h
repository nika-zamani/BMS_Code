/* FreeRTOS includes */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include <string.h>

#define SLAVE_COUNT 1
// change this for daisy chain -> 10

#define ltccsport BSP::gpio::PortB
#define ltccspin 0
#define ltcbaud 500000U

#define tick_ms(x) (x*10)
#define tick_us(x) (x/100)


// adc defines for current
#define ADC_CURRENT_BASE ADC0
#define ADC_CURRENT_CH 8

// gpio defines for imd ok
#define GPIO_IMD_OK_PORT gpio::PortD
#define GPIO_IMD_OK_CH 15

// gpio defines for bms ok
#define GPIO_BMS_OK_PORT gpio::PortD
#define GPIO_BMS_OK_CH 15

// gpio defines for bms airs
#define GPIO_BMS_AIRS_PORT gpio::PortD
#define GPIO_BMS_AIRS_CH 15

// can
#define VOLTAGE_ID 0x300
#define TEMP_ID 0x314
#define OK_ID 0x334
#define MAIN_ID 0x335
#define AIRS_ID 0x336

#define CAN_BUS 0
#define CAN_BAUD_RATE 1000000
