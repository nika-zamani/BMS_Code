
#include "taskManager.h"
#include "gpio.h"

extern BMS bms;

void taskBmsInfo(void *)
{
    TickType_t xLastWakeTime;
    bmsInit();

    // Delay to let boards go into standby
    vTaskDelay(pdMS_TO_TICKS(2000));

    for (;;)
    {
        xLastWakeTime = xTaskGetTickCount();

        // BMS 
        getVoltages(0b10);
        getTempuratures(0b10);
        calculateBMS_OK();
        setBMS_OK();
        sendVoltages();
        sendTemperatures();
        //SControl();


        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(100));
    }
}

void taskCanDataCollect(void *)
{
    TickType_t xLastWakeTime;

    for (;;)
    {
        xLastWakeTime = xTaskGetTickCount();

        readGpioInputs();
        readDcdcTemp();
        sendBmsOkTsReadyTsLiveDcdcInfo();

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(100));
    }
}



void taskMainVoltageTempCurrent(void *)
{
    TickType_t xLastWakeTime;

    for (;;)
    {
        xLastWakeTime = xTaskGetTickCount();

        measureSendVoltageTempCurrent();

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(50));
    }
}

void taskInit()
{
    xTaskCreate(transaction, "transaction", STACK_SIZE, NULL, configMAX_PRIORITIES - 1, NULL);

    xTaskCreate(taskDequeueCan, "taskDequeueCan", STACK_SIZE, NULL, configMAX_PRIORITIES - 1, NULL);
    xTaskCreate(taskBmsInfo, "taskBmsInfo", STACK_SIZE*2, NULL, configMAX_PRIORITIES - 1, NULL);
    xTaskCreate(taskCanDataCollect, "taskCanDataCollect", STACK_SIZE*2, NULL, configMAX_PRIORITIES - 1, NULL);


    xTaskCreate(taskMainVoltageTempCurrent, "taskMainVoltageTempCurrent", STACK_SIZE*2, NULL, configMAX_PRIORITIES - 1, NULL);
}