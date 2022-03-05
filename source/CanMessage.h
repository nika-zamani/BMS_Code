#ifndef CAN_MESSAGE_H
#define CAN_MESSAGE_H

#include "can.h"
#include "taskManager.h"
#include "bms.h"
#include "io.h"
#include "common.h"

using namespace BSP;

QueueHandle_t msg_queue = xQueueCreate(20, sizeof(can::CANlight::frame));

void cb(void);
void canInit();
void canSend(uint8_t bus, uint32_t addr, uint64_t *data);
void sendVoltage(uint16_t cellVoltage[8], int id);
void sendTemp(uint16_t thermistorValues[8], int id);
void sendMainVoltageTempCurrent(uint16_t voltage, uint16_t maxTemp, uint16_t current);

void sendBmsOkTsReadyTsLiveDcdcInfo(uint8_t bms_ok, uint8_t ts_ready, uint8_t ts_live, uint8_t dcdc_fault, uint16_t dcdc_temp);

#endif