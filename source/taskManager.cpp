
#include "taskManager.h"

extern BMS bms;

void taskBmsInfo(void *)
{
    TickType_t xLastWakeTime;
    for (;;)
    {
        xLastWakeTime = xTaskGetTickCount();
        uint32_t maxVoltTemp = calcVoltFromTemp(35);


        readGpioIn();
        getVoltages(0b10);
        getTempuratures(0b10);
        calculateBMS_OK(maxVoltTemp);
        sendBmsOkTsReadyTsLiveDcdcInfo(bms.output.bms_ok, bms.input.ts_live, bms.input.ts_ready, bms.input.dcdc_fault, bms.input.dcdc_temp);

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(100));
    }
}

void taskGetVoltages(void *)
{
    TickType_t xLastWakeTime;
    const TickType_t period = tick_ms(1000);
    for (;;)
    {
        xLastWakeTime = xTaskGetTickCount();

        getVoltages(0b10);
        sendVoltages();
        
        vTaskDelayUntil(&xLastWakeTime, period);
    }
}

void taskGetTemperatures(void *)
{
    TickType_t xLastWakeTime;
    const TickType_t period = tick_ms(1000);
    for (;;)
    {
        xLastWakeTime = xTaskGetTickCount();

        getTempuratures(0b10);
        sendTemperatures();
        
        vTaskDelayUntil(&xLastWakeTime, period);
    }
}

void taskMainVoltageTempCurrent(void *)
{
    TickType_t xLastWakeTime;
    const TickType_t period = tick_ms(1000);
    for (;;)
    {
        xLastWakeTime = xTaskGetTickCount();

        measureSendVoltageTempCurrent();
        
        vTaskDelayUntil(&xLastWakeTime, period);
    }
}


void taskInit()
{
    xTaskCreate(transaction, "transaction", STACK_SIZE, NULL, configMAX_PRIORITIES-1, NULL );

    xTaskCreate(taskDequeueCan, "taskDequeueCan", STACK_SIZE, NULL, configMAX_PRIORITIES - 1, NULL);
    xTaskCreate(taskBmsInfo, "taskIO", STACK_SIZE, NULL, configMAX_PRIORITIES - 1, NULL);

    xTaskCreate(taskMainVoltageTempCurrent, "taskMainVoltageTempCurrent", STACK_SIZE, NULL, configMAX_PRIORITIES - 1, NULL);

    xTaskCreate(taskGetVoltages, "taskGetVoltages", STACK_SIZE, NULL, configMAX_PRIORITIES - 1, NULL);
    xTaskCreate(taskGetTemperatures, "taskGetTemperatures", STACK_SIZE, NULL, configMAX_PRIORITIES - 1, NULL);


}