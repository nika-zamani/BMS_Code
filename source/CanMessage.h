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
void sendMainVoltageTempCurrent(uint32_t voltage, uint16_t maxTemp, uint16_t current);

void sendBmsOkTsReadyTsLiveDcdcInfo();

void sendChargingCommands(bool on);

typedef struct
{
    uint16_t voltage; // Can use any intx_t uintx_t char bool etc
    uint16_t current;
    uint8_t control;
    uint8_t reserved1;
    uint8_t reserved2;
    uint8_t reserved3;

} ChargingCommand; // Must be 64 bits

#endif