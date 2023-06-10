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

#define VOLTS_TO_BAT_VOLT(V) (V*10000)

const uint8_t ERR_CVA = 1, ERR_CVB = 2, ERR_CVC = 4, ERR_CVD = 8, ERR_AUXA = 16, ERR_AUXB = 32, ERR_STATA = 64, ERR_STATB = 128;

#define initResistance 2
#define vDropThresh 1.4

#define BATTERY_TEMP_LIMIT 45000 // Celcius
#define BATTERY_VOLT_LIMIT_LOWER 22000
#define BATTERY_VOLT_LIMIT_HIGHER 45000


#define DISHARGE_THRESHOLD VOLTS_TO_BAT_VOLT(.01) //.1

#define CHARGING_THRESHOLD 42030

#define ERROR_WAIT_TIME 1500

#define THERMISTOR_CALIBRATION_1 4000
#define THERMISTOR_CALIBRATION_45 4000
#define THERMISTOR_CALIBRATION_2 3500
#define THERMISTOR_CALIBRATION_3 4000

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
void SControl();

#endif 
