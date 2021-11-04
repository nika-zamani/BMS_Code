#ifndef CAN_MESSAGE_H
#define CAN_MESSAGE_H


#include "can.h"


class CanMessage {

public:
    static void sendVoltage(uint16_t cellVoltage[12], int id);
    static void sendTemp(uint16_t thermistorValues[16], int id);

private:
    static CanMessage *getInstance();
    void sendVoltageHelper(uint16_t cellVoltage[12], int id);
    void sendTempHelper(uint16_t thermistorValues[16], int id);

    CanMessage();
    void initCan();
    static CanMessage* instance;
};


#endif