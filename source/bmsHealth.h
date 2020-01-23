#ifndef BMSHEALTH_H
#define BMSHEALTH_H

#include "common.h"
#include "bmsCommand.h"
#include "transaction.h"

const uint8_t ERR_CVA = 1, ERR_CVB = 2, ERR_CVC = 4, ERR_CVD = 8, ERR_AUXA = 16, ERR_AUXB = 32, ERR_STATA = 64, ERR_STATB = 128;

// to include test task, add to main:
// xTaskCreate(monitorBMSHealth, "monitorbmshlth", STACK_SIZE, NULL, 1, NULL );
void monitorBMSHealth(void *pvParameters);

#endif // BMSHEALTH_H


