
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

void cb(void) {
    can::CANlight &can = can::CANlight::StaticClass();
    can::CANlight::frame f = can.readrx(CAN_BUS); // or is it readrx(0)?

    /* Loop through structs */
    xQueueSendFromISR(msg_queue, &f, pdFALSE);

}

void CanMessage::initCan() {
    

    // Fix can priority to allow RTOS interupts during can callbacks
    NVIC->IP[78] |= 6 << 4;
    NVIC->IP[79] |= 6 << 4;
    NVIC->IP[80] |= 6 << 4;
    NVIC->IP[81] |= 6 << 4;

    NVIC->IP[85] |= 6 << 4;
    NVIC->IP[86] |= 6 << 4;
    NVIC->IP[87] |= 6 << 4;
    NVIC->IP[88] |= 6 << 4;

    can::can_config c;
	can::CANlight::ConstructStatic(&c);
	can::CANlight& can = can::CANlight::StaticClass();
    
	can::CANlight::canx_config cx;
    //cx.callback = cb;
    cx.globalMaskValue = 0b00000010000000000000000000000;
    cx.baudRate = CAN_BAUD_RATE;
    can.init(0, &cx);

}


void CanMessage::sendVoltageHelper(uint16_t cellVoltage[8], int id) {
    
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
    frame1.id = (VOLTAGE_ID + (2 * id)) | MEDIUM_CAN_PRIORITY;
    frame1.ext = 1;
    frame1.dlc = 8;
    memcpy(frame1.data, &voltageCanstruct0, sizeof(BmsTempStruct));
    can1.tx(CAN_BUS, frame1);

    can::CANlight &can2 = can::CANlight::StaticClass();
    can::CANlight::frame frame2;
    frame2.id = (VOLTAGE_ID + (2 * id) + 1) | MEDIUM_CAN_PRIORITY;
    frame2.ext = 1;
    frame2.dlc = 8;
    memcpy(frame2.data, &voltageCanstruct1, sizeof(BmsTempStruct));
    can2.tx(CAN_BUS, frame2);
}

void CanMessage::sendTempHelper(uint16_t thermistorValues[8], int id) {
    
    BmsTempStruct temperatureCanstruct0;
    BmsTempStruct temperatureCanstruct1;

    temperatureCanstruct0.voltage0 = thermistorValues[0];
    temperatureCanstruct0.voltage1 = thermistorValues[1];
    temperatureCanstruct0.voltage2 = thermistorValues[2];
    temperatureCanstruct0.voltage3 = thermistorValues[3];

    temperatureCanstruct1.voltage0 = thermistorValues[4];
    temperatureCanstruct1.voltage1 = thermistorValues[5];
    temperatureCanstruct1.voltage2 = thermistorValues[6];
    temperatureCanstruct1.voltage3 = thermistorValues[7];

    can::CANlight &can1 = can::CANlight::StaticClass();
    can::CANlight::frame frame1;
    frame1.id = (TEMP_ID + (2 * id)) | MEDIUM_CAN_PRIORITY;
    frame1.ext = 1;
    frame1.dlc = 8;
    memcpy(frame1.data, &temperatureCanstruct0, sizeof(BmsTempStruct));
    can1.tx(CAN_BUS, frame1);

    can::CANlight &can2 = can::CANlight::StaticClass();
    can::CANlight::frame frame2;
    frame2.id = (TEMP_ID + (2 * id) + 1) | MEDIUM_CAN_PRIORITY;
    frame2.ext = 1;
    frame2.dlc = 8;
    memcpy(frame2.data, &temperatureCanstruct1, sizeof(BmsTempStruct));
    can2.tx(CAN_BUS, frame2);
}

void CanMessage::sendImdBmsOkHelper(uint8_t BMS_OK, uint8_t IMD_OK) {
    // bool of imd ok and bms ok
    BmsOkStruct okStruct;
    memset(&okStruct, 0, sizeof(BmsOkStruct));

    okStruct.bms_ok = BMS_OK;
    okStruct.imd_ok = IMD_OK;

    can::CANlight &can = can::CANlight::StaticClass();
    can::CANlight::frame frame;

    frame.id = OK_ID | HIGH_CAN_PRIORITY | VCU_CAN_TARGET;
    frame.ext = 1;
    frame.dlc = 8;
    memcpy(frame.data, &okStruct, sizeof(BmsOkStruct));
    can.tx(CAN_BUS, frame);

}

void CanMessage::sendMainVoltageTempCurrentHelper(uint16_t voltage, uint16_t maxTemp, uint16_t current) {
    BmsMainVoltageTempCurrentStruct mainVoltageTempCurrentStruct;
    memset(&mainVoltageTempCurrentStruct, 0, sizeof(BmsMainVoltageTempCurrentStruct));

    mainVoltageTempCurrentStruct.voltage = voltage;
    mainVoltageTempCurrentStruct.maxTemp = maxTemp;
    mainVoltageTempCurrentStruct.current = current;

    can::CANlight &can = can::CANlight::StaticClass();
    can::CANlight::frame frame;

    frame.id = MAIN_ID | CRITICAL_CAN_PRIORITY | VCU_CAN_TARGET;
    frame.ext = 1;
    frame.dlc = 8;
    memcpy(frame.data, &mainVoltageTempCurrentStruct, sizeof(mainVoltageTempCurrentStruct));
    can.tx(CAN_BUS, frame);
}

void CanMessage::sendMainVoltageTempCurrent(uint16_t voltage, uint16_t maxTemp, uint16_t current) {
    CanMessage *c = CanMessage::getInstance();
    c->sendMainVoltageTempCurrentHelper(voltage, maxTemp, current);
}

void CanMessage::sendImdBmsOk(uint8_t BMS_OK, uint8_t IMD_OK) {
    CanMessage *c = CanMessage::getInstance();
    c->sendImdBmsOkHelper(BMS_OK, IMD_OK);
}

void CanMessage::sendVoltage(uint16_t cellVoltage[8], int id) {
    CanMessage *c = CanMessage::getInstance();
    c->sendVoltageHelper(cellVoltage, id);
}

// bools take up 1 byte

void CanMessage::sendTemp(uint16_t thermistorValues[8], int id) {
    CanMessage *c = CanMessage::getInstance();
    c->sendTempHelper(thermistorValues, id);
}