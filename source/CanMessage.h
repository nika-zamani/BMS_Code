#ifndef CAN_MESSAGE_H
#define CAN_MESSAGE_H


#include "can.h"
#include "taskManager.h"

QueueHandle_t msg_queue = xQueueCreate(20, sizeof(can::CANlight::frame));

class CanMessage {

public:
    static void sendVoltage(uint16_t cellVoltage[8], int id);
    static void sendTemp(uint16_t thermistorValues[8], int id);
    static void sendImdBmsOk(uint8_t BMS_OK, uint8_t IMD_OK);
    static void sendMainVoltageTempCurrent(uint16_t voltage, uint16_t maxTemp, uint16_t current);

    static CanMessage *getInstance();
private:
    
    void sendVoltageHelper(uint16_t cellVoltage[8], int id);
    void sendTempHelper(uint16_t thermistorValues[8], int id);
    void sendImdBmsOkHelper(uint8_t BMS_OK, uint8_t IMD_OK);
    void sendMainVoltageTempCurrentHelper(uint16_t voltage, uint16_t maxTemp, uint16_t current);
    void canSend(uint8_t bus, uint32_t addr, uint64_t *data);

    CanMessage();
    void initCan();
    static CanMessage* instance;
};


#endif