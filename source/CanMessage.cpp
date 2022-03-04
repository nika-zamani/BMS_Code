#include "FreeRTOS.h"
#include "task.h"

#include "CanMessage.h"
#include "main.h"
#include "canmessagestructs.h"

using namespace BSP;

CanMessage *CanMessage::instance;

CanMessage::CanMessage() {}

CanMessage *CanMessage::getInstance()
{
    return instance;
}

void cb(void)
{
    can::CANlight &can = can::CANlight::StaticClass();
    can::CANlight::frame f = can.readrx(CAN_BUS);

    /* Loop through structs */
    xQueueSendFromISR(msg_queue, &f, pdFALSE);
}

void initCan()
{
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
    can::CANlight &can = can::CANlight::StaticClass();

    can::CANlight::canx_config cx;
    cx.callback = cb;
    cx.globalMaskValue = 0b00000010000000000000000000000;
    cx.baudRate = CAN_BAUD_RATE;
    can.init(0, &cx);
}

void canSend(uint8_t bus, uint32_t address, uint64_t *data)
{
    can::CANlight &can = can::CANlight::StaticClass();
    can::CANlight::frame frame;

    frame.id = address;
    frame.ext = 1;
    frame.dlc = 8;
    memcpy(frame.data, data, sizeof(frame.data));

    can.tx(bus, frame);
}

void CanMessage::sendVoltage(uint16_t cellVoltage[8], int id)
{
    BmsVoltageStruct voltageCanstruct0;
    BmsVoltageStruct voltageCanstruct1;

    voltageCanstruct0.voltage0 = cellVoltage[0]; // first 4 cells
    voltageCanstruct0.voltage1 = cellVoltage[1];
    voltageCanstruct0.voltage2 = cellVoltage[2];
    voltageCanstruct0.voltage3 = cellVoltage[3];

    voltageCanstruct1.voltage0 = cellVoltage[4]; // last 4 cells
    voltageCanstruct1.voltage1 = cellVoltage[5];
    voltageCanstruct1.voltage2 = cellVoltage[6];
    voltageCanstruct1.voltage3 = cellVoltage[7];

    canSend(CAN_BUS, (VOLTAGE_ID + (2 * id)) | MEDIUM_CAN_PRIORITY, (uint64_t *)&voltageCanstruct0);
    canSend(CAN_BUS, (VOLTAGE_ID + (2 * id) + 1) | MEDIUM_CAN_PRIORITY, (uint64_t *)&voltageCanstruct1);
}

void CanMessage::sendTemp(uint16_t thermistorValues[8], int id)
{

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

    canSend(CAN_BUS, (TEMP_ID + (2 * id)) | MEDIUM_CAN_PRIORITY, (uint64_t *)&temperatureCanstruct0);
    canSend(CAN_BUS, (TEMP_ID + (2 * id) + 1) | MEDIUM_CAN_PRIORITY, (uint64_t *)&temperatureCanstruct1);
}

void sendBmsOkTsReadyTsLiveDcdcInfo(uint8_t bms_ok, uint8_t ts_ready, uint8_t ts_live, uint8_t dcdc_fault, uint16_t dcdc_temp)
{
    BmsOkTSLiveDCDCStruct data;
    memset(&data, 0, sizeof(BmsOkTSLiveDCDCStruct));

    data.bms_ok = BMS_OK;
    data.ts_ready = ts_ready;
    data.ts_live = ts_live;
    data.dcdc_fault = dcdc_fault;
    data.dcdc_temp = 
    data.unused0 = 0;

    canSend(CAN_BUS, OK_ID | HIGH_CAN_PRIORITY | VCU_CAN_TARGET, (uint64_t *)&data);
}

void CanMessage::sendMainVoltageTempCurrent(uint16_t voltage, uint16_t maxTemp, uint16_t current)
{
    BmsMainVoltageTempCurrentStruct mainVoltageTempCurrentStruct;
    memset(&mainVoltageTempCurrentStruct, 0, sizeof(BmsMainVoltageTempCurrentStruct));

    mainVoltageTempCurrentStruct.voltage = voltage;
    mainVoltageTempCurrentStruct.maxTemp = maxTemp;
    mainVoltageTempCurrentStruct.current = current;

    canSend(CAN_BUS, MAIN_ID | CRITICAL_CAN_PRIORITY | VCU_CAN_TARGET, (uint64_t *)&mainVoltageTempCurrentStruct);
}
