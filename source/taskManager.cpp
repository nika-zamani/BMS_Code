
#include "taskManager.h"
#include "gpio.h"

extern BMS bms;

void taskBmsInfo(void *)
{
    TickType_t xLastWakeTime;
    bmsInit();

    uint8_t counter = 0;
    gpio::GPIO &gpio = gpio::GPIO::StaticClass();

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
        
        if (counter == 10 && bms.input.is_charging){
            SControl();
            bms.input.ts_ready ? gpio.set(GPIO_AIR_POS_PORT, GPIO_AIR_POS_CH) : gpio.clear(GPIO_AIR_POS_PORT, GPIO_AIR_POS_CH);
            bms.input.ts_ready ? gpio.set(GPIO_AIR_NEG_PORT, GPIO_AIR_NEG_CH) : gpio.clear(GPIO_AIR_NEG_PORT, GPIO_AIR_NEG_CH);
            sendChargingCommands(true);   
            counter = 0;
        }
        else
        {
            counter +=1;
        }


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