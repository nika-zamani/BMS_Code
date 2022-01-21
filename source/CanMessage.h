#ifndef CAN_MESSAGE_H
#define CAN_MESSAGE_H


#include "can.h"
#include "taskManager.h"

QueueHandle_t msg_queue = xQueueCreate(20, sizeof(can::CANlight::frame));

class CanMessage {

public:
    static void sendVoltage(uint16_t cellVoltage[12], int id);
    static void sendTemp(uint16_t thermistorValues[16], int id);
    static void sendImdBmsOk(uint8_t BMS_OK, uint8_t IMD_OK);
    static void sendMainVoltageCurrent(uint16_t voltage, uint16_t current);


private:
    static CanMessage *getInstance();
    void sendVoltageHelper(uint16_t cellVoltage[12], int id);
    void sendTempHelper(uint16_t thermistorValues[16], int id);
    void sendImdBmsOkHelper(uint8_t BMS_OK, uint8_t IMD_OK);
    void sendMainVoltageCurrentHelper(uint16_t voltage, uint16_t current);

    CanMessage();
    void initCan();
    static CanMessage* instance;
};


#endif