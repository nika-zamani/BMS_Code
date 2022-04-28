#ifndef BMSHEALTH_H
#define BMSHEALTH_H

#include <math.h>

#include "common.h"
#include "io.h"
#include "bmsCommand.h"
#include "transaction.h"
#include "bms.h"
#include "canmessagestructs.h"
#include "CanMessage.h"

using namespace BSP;

const uint8_t ERR_CVA = 1, ERR_CVB = 2, ERR_CVC = 4, ERR_CVD = 8, ERR_AUXA = 16, ERR_AUXB = 32, ERR_STATA = 64, ERR_STATB = 128;

#define initResistance 2
#define vDropThresh 1.4

#define BATTERY_TEMP_VOLT_LIMIT calcTempToVolt(35) // Celcius

uint32_t calcTempToVolt(uint16_t temperature);
uint16_t calcVoltToTemp(uint16_t voltage);
uint16_t getMaxTemp();
uint32_t getSumVoltage();
void getVoltages(uint8_t md);
void getTempuratures(uint8_t md);
void calculateBMS_OK();
void sendVoltages();
void sendTemperatures();
void measureSendVoltageTempCurrent();
void bmsInit();
void monitorBMSHealth(void *pvParameters);

#endif 
