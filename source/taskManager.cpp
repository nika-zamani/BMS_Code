#include "adc.h"

#include "CanMessage.h"
#include "taskManager.h"
#include "common.h"
#include "bmsHealth.h"
#include "canmessagestructs.h"

using namespace BSP;

uint8_t TS_LIVE = false;
uint8_t TS_READY = false;
uint8_t DCDC_FAULT = false;

void not_found_id()
{
}

void unpackCanAirPrechargeDcdcEnable(can::CANlight::frame *f)
{
    BmsAirsPreChargeDCDCEnableStruct data;
    gpio::GPIO &gpio = gpio::GPIO::StaticClass();

    memcpy(&data, &f, sizeof(BmsAirsPreChargeDCDCEnableStruct));
    data.airs_positive ? gpio.set(GPIO_AIR_POS_PORT, GPIO_AIR_POS_CH) : gpio.clear(GPIO_AIR_POS_PORT, GPIO_AIR_POS_CH);
    data.airs_negative ? gpio.set(GPIO_AIR_NEG_PORT, GPIO_AIR_NEG_CH) : gpio.clear(GPIO_AIR_NEG_PORT, GPIO_AIR_NEG_CH);
    data.precharge ? gpio.set(GPIO_PRECHARGE_PORT, GPIO_PRECHARGE_CH) : gpio.clear(GPIO_PRECHARGE_PORT, GPIO_PRECHARGE_CH);
    data.dcdc_enable ? gpio.set(GPIO_DCDC_EN_PORT, GPIO_DCDC_EN_CH) : gpio.clear(GPIO_DCDC_EN_PORT, GPIO_DCDC_EN_CH);
}

void trans(void *pvParameters)
{
    can::CANlight::frame receiveCommand;
    for (;;)
    {
        xQueueReceive(msg_queue, (void *)&receiveCommand, portMAX_DELAY);
        int i = 1;
        switch (receiveCommand.id & 0xFFF)
        {
        case AIRS_ID:
            unpackCanAirPrechargeDcdcEnable(&receiveCommand);
            break;
        default:
            not_found_id();
        }
    }
}

uint16_t getDcdcTemp()
{
    adc::ADC &adc = adc::ADC::StaticClass();
    uint16_t tempADC = adc.read(ADC_DCDC_TEMP_BASE, ADC_DCDC_TEMP_CH);
    return ((tempADC / 4095.0 * 5.0)) * 50.76142 * 1000;
}

void readGpioIn()
{
    gpio::GPIO &gpio = gpio::GPIO::StaticClass();

    TS_LIVE = gpio.read(GPIO_TS_LIVE_PORT, GPIO_TS_LIVE_CH);
    TS_READY = gpio.read(GPIO_TS_READY_PORT, GPIO_TS_READY_CH);
    DCDC_FAULT = gpio.read(GPIO_DCDC_FAULT_PORT, GPIO_DCDC_FAULT_CH);
}

void taskStateMachineUpdate(void *)
{
    TickType_t xLastWakeTime;
    for (;;)
    {
        xLastWakeTime = xTaskGetTickCount();

        readGpioIn();
        sendBmsOkTsReadyTsLiveDcdcInfo(BMS_OK, TS_LIVE, TS_READY, DCDC_FAULT, getDcdcTemp());

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(100));
    }
}

void transInit()
{
    xTaskCreate(trans, "trans", STACK_SIZE, NULL, configMAX_PRIORITIES - 1, NULL);
}