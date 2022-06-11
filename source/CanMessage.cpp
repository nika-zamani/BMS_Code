#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#include "CanMessage.h"
#include "main.h"
#include "canmessagestructs.h"

extern BMS bms;

//static SemaphoreHandle_t canMutex;

void cb(void)
{
    can::CANlight &can = can::CANlight::StaticClass();
    can::CANlight::frame f = can.readrx(CAN_BUS);

    /* Loop through structs */
    xQueueSendFromISR(msg_queue, &f, pdFALSE);
}

void canInit()
{
    NVIC->IP[78] |= 6 << 4;
    NVIC->IP[79] |= 6 << 4;
    NVIC->IP[80] |= 6 << 4;
    NVIC->IP[81] |= 6 << 4;

    NVIC->IP[85] |= 6 << 4;
    NVIC->IP[86] |= 6 << 4;
    NVIC->IP[87] |= 6 << 4;
    NVIC->IP[88] |= 6 << 4;

    //canMutex = xSemaphoreCreateMutex();
    // Take the mutex
    //xSemaphoreTake(canMutex, portMAX_DELAY);

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

    //if (xSemaphoreTake(canMutex, portMAX_DELAY) == pdTRUE){
        can.tx(bus, frame);
    //   xSemaphoreGive(canMutex);
    //}
}

void sendVoltage(uint16_t cellVoltage[8], int id)
{
    BmsVoltageStruct voltageCanstruct0;
    BmsVoltageStruct voltageCanstruct1;

    voltageCanstruct0.voltage0 = cellVoltage[0]; // first 4 cells
    voltageCanstruct0.voltage1 = cellVoltage[1];
    voltageCanstruct0.voltage2 = cellVoltage[2];
    voltageCanstruct0.voltage3 = cellVoltage[3];
    canSend(CAN_BUS, (VOLTAGE_ID + (2 * id)) | NO_TARGET | MEDIUM_CAN_PRIORITY, (uint64_t *)&voltageCanstruct0);


    voltageCanstruct1.voltage0 = cellVoltage[4]; // last 4 cells
    voltageCanstruct1.voltage1 = cellVoltage[5];
    voltageCanstruct1.voltage2 = cellVoltage[6];
    voltageCanstruct1.voltage3 = cellVoltage[7];

    canSend(CAN_BUS, (VOLTAGE_ID + (2 * id) + 1) | NO_TARGET | MEDIUM_CAN_PRIORITY, (uint64_t *)&voltageCanstruct1);
}

void sendTemp(uint16_t thermistorValues[8], int id)
{

    BmsTempStruct temperatureCanstruct0;
    BmsTempStruct temperatureCanstruct1;

    temperatureCanstruct0.voltage0 = thermistorValues[0];
    temperatureCanstruct0.voltage1 = thermistorValues[1];
    temperatureCanstruct0.voltage2 = thermistorValues[2];
    temperatureCanstruct0.voltage3 = thermistorValues[3];
    uint32_t tenp = (TEMP_ID + (2 * id)) | NO_TARGET |  MEDIUM_CAN_PRIORITY;
    canSend(CAN_BUS, (TEMP_ID + (2 * id)) | NO_TARGET |  MEDIUM_CAN_PRIORITY, (uint64_t *)&temperatureCanstruct0);

    temperatureCanstruct1.voltage0 = thermistorValues[4];
    temperatureCanstruct1.voltage1 = thermistorValues[5];
    temperatureCanstruct1.voltage2 = thermistorValues[6];
    temperatureCanstruct1.voltage3 = thermistorValues[7];

    canSend(CAN_BUS, (TEMP_ID + (2 * id) + 1) | NO_TARGET |  MEDIUM_CAN_PRIORITY, (uint64_t *)&temperatureCanstruct1);
}

void sendBmsOkTsReadyTsLiveDcdcInfo()
{
    BmsOkTSLiveDCDCStruct data;
    memset(&data, 0, sizeof(BmsOkTSLiveDCDCStruct));

    data.bms_ok = bms.output.bms_ok;
    data.ts_ready = bms.input.ts_ready;
    data.ts_live = bms.input.ts_live;
    data.dcdc_fault = bms.input.dcdc_fault;
    data.dcdc_temp = bms.input.dcdc_temp;
    data.unused0 = 0;

    canSend(CAN_BUS, OK_ID | CRITICAL_CAN_PRIORITY | VCU_CAN_TARGET, (uint64_t *)&data);
}

void sendMainVoltageTempCurrent(uint32_t voltage, uint16_t maxTemp, uint16_t current)
{
    BmsMainVoltageTempCurrentStruct mainVoltageTempCurrentStruct;
    memset(&mainVoltageTempCurrentStruct, 0, sizeof(BmsMainVoltageTempCurrentStruct));

    mainVoltageTempCurrentStruct.voltage = voltage;
    mainVoltageTempCurrentStruct.maxTemp = maxTemp;
    mainVoltageTempCurrentStruct.current = current;

    canSend(CAN_BUS, MAIN_ID | CRITICAL_CAN_PRIORITY | VCU_CAN_TARGET, (uint64_t *)&mainVoltageTempCurrentStruct);
}

void sendChargingCommands(bool on){
    ChargingCommand chargingCommand;
    memset(&chargingCommand, 0, sizeof(ChargingCommand));

    chargingCommand.voltage = 3360;
    chargingCommand.current = 10;
    chargingCommand.control = !on;

    canSend(CAN_BUS, CHARGING_COMMAND_ID,  (uint64_t *)&chargingCommand);
}
