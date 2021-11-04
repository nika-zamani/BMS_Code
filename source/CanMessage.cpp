
#include "CanMessage.h"
#include "FreeRTOS.h"
#include "task.h"
#include "can.h"
#include "main.h"
#include "../CanMessageStructs/canmessagestructs.h"

// #include "StateMachine.h"

using namespace BSP;

CanMessage* CanMessage::instance;

CanMessage::CanMessage(){}

CanMessage* CanMessage::getInstance() {
    if(instance == 0){
        instance = new CanMessage();
        instance->initCan();
    }
    return instance;
}


void CanMessage::initCan() {
    can::can_config config;
    can::CANlight::canx_config canx_config;

    // Initialize CAN driver
    can::CANlight::ConstructStatic(&config);
    can::CANlight &can = can::CANlight::StaticClass();

    // Configure CAN bus
    canx_config.baudRate = CAN_BAUD_RATE;
    can.init(CAN_BUS, &canx_config);
}


void CanMessage::sendVoltageHelper(uint16_t cellVoltage[12], int id) {
    
    BmsVoltageStruct voltageCanstruct0;
    BmsVoltageStruct voltageCanstruct1;

    voltageCanstruct0.voltage0 = cellVoltage[0];    // first 4 cells
    voltageCanstruct0.voltage1 = cellVoltage[1];
    voltageCanstruct0.voltage2 = cellVoltage[2];
    voltageCanstruct0.voltage3 = cellVoltage[3];

    voltageCanstruct1.voltage0 = cellVoltage[4];    // last 4 cells
    voltageCanstruct1.voltage1 = cellVoltage[5];
    voltageCanstruct1.voltage2 = cellVoltage[6];
    voltageCanstruct1.voltage3 = cellVoltage[7];

    can::CANlight &can1 = can::CANlight::StaticClass();
    can::CANlight::frame frame1;
    frame1.id = VOLTAGE_ID + (2 * id);
    frame1.ext = 1;
    frame1.dlc = 8;
    memcpy(frame1.data, &voltageCanstruct0, sizeof(BmsTempStruct));
    can1.tx(CAN_BUS, frame1);

    can::CANlight &can2 = can::CANlight::StaticClass();
    can::CANlight::frame frame2;
    frame2.id = VOLTAGE_ID + (2 * id) + 1;
    frame2.ext = 1;
    frame2.dlc = 8;
    memcpy(frame2.data, &voltageCanstruct1, sizeof(BmsTempStruct));
    can2.tx(CAN_BUS, frame2);
}

void CanMessage::sendTempHelper(uint16_t thermistorValues[16], int id) {
    
    BmsTempStruct temperatureCanstruct0;
    BmsTempStruct temperatureCanstruct1;

    temperatureCanstruct0.voltage0 = thermistorValues[0];
    temperatureCanstruct0.voltage1 = thermistorValues[2];
    temperatureCanstruct0.voltage2 = thermistorValues[4];
    temperatureCanstruct0.voltage3 = thermistorValues[6];

    temperatureCanstruct1.voltage0 = thermistorValues[8];
    temperatureCanstruct1.voltage1 = thermistorValues[10];
    temperatureCanstruct1.voltage2 = thermistorValues[12];
    temperatureCanstruct1.voltage3 = thermistorValues[14];

    can::CANlight &can1 = can::CANlight::StaticClass();
    can::CANlight::frame frame1;
    frame1.id = TEMP_ID + (2 * id);
    frame1.ext = 1;
    frame1.dlc = 8;
    memcpy(frame1.data, &temperatureCanstruct0, sizeof(BmsTempStruct));
    can1.tx(CAN_BUS, frame1);

    can::CANlight &can2 = can::CANlight::StaticClass();
    can::CANlight::frame frame2;
    frame2.id = TEMP_ID + (2 * id) + 1;
    frame2.ext = 1;
    frame2.dlc = 8;
    memcpy(frame2.data, &temperatureCanstruct1, sizeof(BmsTempStruct));
    can2.tx(CAN_BUS, frame2);
}

void CanMessage::sendImdBmsOkHelper(uint8_t BMS_OK, uint8_t IMD_OK) {
    // bool of imd ok and bms ok
    BmsOkStruct okStruct;

    okStruct.bms_ok = BMS_OK;
    okStruct.imd_ok = IMD_OK;

    can::CANlight &can = can::CANlight::StaticClass();
    can::CANlight::frame frame;

    frame.id = OK_ID;
    frame.ext = 1;
    frame.dlc = 8;
    memcpy(frame.data, &okStruct, sizeof(BmsOkStruct));
    can.tx(CAN_BUS, frame);

}

void CanMessage::sendImdBmsOk(uint8_t BMS_OK, uint8_t IMD_OK) {
    CanMessage *c = CanMessage::getInstance();
    c->sendImdBmsOkHelper(BMS_OK, IMD_OK);
}

void CanMessage::sendVoltage(uint16_t cellVoltage[12], int id) {
    CanMessage *c = CanMessage::getInstance();
    c->sendVoltageHelper(cellVoltage, id);
}

// bools take up 1 byte

void CanMessage::sendTemp(uint16_t thermistorValues[16], int id) {
    CanMessage *c = CanMessage::getInstance();
    c->sendTempHelper(thermistorValues, id);
}