#ifndef MAIN_H
#define MAIN_H

/* Hardware specific includes */
#include "MKE18F16.h"
#include "clock_config.h"
#include "pin_mux.h"

#include "bmsCommand.h"
#include "transaction.h"
#include "taskManager.h"

#include "testroutines.h"
#include "bmsHealth.h"
#include "bms.h"
#include "CanMessage.h"
#include "io.h"
#include "common.h"

#include "gpio.h"
#include "spi.h"
#include "adc.h"

/* The configCHECK_FOR_STACK_OVERFLOW setting in FreeRTOSConifg can be used to
 * check task stacks for overflows.  It does not however check the stack used by
 * interrupts.  This demo has a simple addition that will also check the stack used
 * by interrupts if mainCHECK_INTERRUPT_STACK is set to 1.  Note that this check is
 * only performed from the tick hook function (which runs in an interrupt context).
 * It is a good debugging aid - but won't catch interrupt stack problems until the
 * tick interrupt next executes. */
#define mainCHECK_INTERRUPT_STACK 0
#if mainCHECK_INTERRUPT_STACK == 1
const unsigned char ucExpectedInterruptStackValues[] = {0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC};
#endif

#define STACK_SIZE 100

#endif
