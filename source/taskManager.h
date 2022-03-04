#ifndef TASKMANAGER_H
#define TASKMANAGER_H

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"
#include "gpio.h"
#include "adc.h"
#include "MKE18F16.h"
#include "clock_config.h"
#include "pin_mux.h"
#include "bms.h"
#include "bmsHealth.h"
#include "CanMessage.h"
#include "io.h"
#include "can.h"
#include "canmessagestructs.h"
#include "transaction.h"
#include "common.h"

using namespace BSP;

#define STACK_SIZE 100

void taskInit();
void taskGetVoltages(void *);
void taskGetTemperatures(void *);

#endif