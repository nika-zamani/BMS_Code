#ifndef TASKMANAGER_H
#define TASKMANAGER_H

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"
#include "gpio.h"
#include "MKE18F16.h"
#include "clock_config.h"
#include "pin_mux.h"

#include "can.h"
using namespace BSP;

#define STACK_SIZE 100

void taskInit();

#endif