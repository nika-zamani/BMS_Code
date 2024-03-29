#ifndef IO_H
#define IO_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include <string.h>
#include "common.h"
#include "MKE18F16.h"
#include "bms.h"
#include "adc.h"
#include "can.h"
#include "gpio.h"
#include "spi.h"
#include "pin_mux.h"
#include "CanMessage.h"
#include "canmessagestructs.h"

using namespace BSP;

#define ltccsport BSP::gpio::PortB
#define ltccspin 0
#define ltcbaud 500000U

// ADC
#define ADC_CURRENT_BASE ADC0
#define ADC_CURRENT_CH 11
#define ADC_DCDC_TEMP_BASE ADC2
#define ADC_DCDC_TEMP_CH 7

// GPIO in
#define GPIO_TS_LIVE_PORT gpio::PortD
#define GPIO_TS_LIVE_CH 0
#define GPIO_TS_READY_PORT gpio::PortE
#define GPIO_TS_READY_CH 3
#define GPIO_DCDC_FAULT_PORT gpio::PortD
#define GPIO_DCDC_FAULT_CH 1

// GPIO out
#define GPIO_BMS_OK_PORT gpio::PortD
#define GPIO_BMS_OK_CH 15
#define GPIO_AIR_POS_PORT gpio::PortA
#define GPIO_AIR_POS_CH 2
#define GPIO_AIR_NEG_PORT gpio::PortA
#define GPIO_AIR_NEG_CH 3
#define GPIO_PRECHARGE_PORT gpio::PortE
#define GPIO_PRECHARGE_CH 9
#define GPIO_DCDC_EN_PORT gpio::PortD
#define GPIO_DCDC_EN_CH 16

// CAN
#define VOLTAGE_ID 0x300
#define TEMP_ID 0x314
#define OK_ID 0x334
#define MAIN_ID 0x335
#define AIRS_ID 0x336
#define LOWEST_VOLT_ID 0x337

#define CHARGING_ID 0x666
#define CHARGING_COMMAND_ID 0x1806E5F4

#define CAN_BUS 0
#define CAN_BAUD_RATE 1000000

void prvSetupHardware(void);
uint16_t measureCurrent();
void unpackCanAirPrechargeDcdcEnable(can::CANlight::frame *f);
void readDcdcTemp();
void readGpioInputs();
void not_found_id();
void taskDequeueCan(void *pvParameters);
void setBMS_OK();

#endif