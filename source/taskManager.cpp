
#include "taskManager.h"
#include "gpio.h"

extern BMS bms;

void taskBmsInfo(void *)
{
    TickType_t xLastWakeTime;
    bmsInit();

    u_int8_t is_actively_charging = false;
    static uint8_t charged = (bms.input.highest_volt >= CHARGING_THRESHOLD) ? 1 : 0;

    //uint8_t counter = 0;
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
        

        // // Discharching
        //SControl();

        // // Charging
        // if (!charged) {
        //     if (bms.input.is_charging && bms.input.ts_ready){
        //         if (is_actively_charging){
        //             gpio.set(GPIO_AIR_NEG_PORT, GPIO_AIR_NEG_CH);
        //             gpio.set(GPIO_AIR_POS_PORT, GPIO_AIR_POS_CH);
        //             gpio.set(GPIO_DCDC_EN_PORT, GPIO_DCDC_EN_CH);
        //             gpio.clear(GPIO_PRECHARGE_PORT, GPIO_PRECHARGE_CH);
        //             sendChargingCommands(true);
        //         }
        //         else
        //         {
        //             gpio.set(GPIO_AIR_NEG_PORT, GPIO_AIR_NEG_CH);
        //             gpio.set(GPIO_PRECHARGE_PORT, GPIO_PRECHARGE_CH);
        //             gpio.set(GPIO_DCDC_EN_PORT, GPIO_DCDC_EN_CH);
        //             pdMS_TO_TICKS(50);
        //             sendChargingCommands(true);
        //             vTaskDelay(pdMS_TO_TICKS(5000)); // Precharge for 5 seconds
        //             is_actively_charging = true;
        //         }
        //     }
        //     else
        //     {
        //         sendChargingCommands(false);
        //         gpio.clear(GPIO_AIR_NEG_PORT, GPIO_AIR_NEG_CH);
        //         gpio.clear(GPIO_AIR_POS_PORT, GPIO_AIR_POS_CH);
        //         gpio.clear(GPIO_PRECHARGE_PORT, GPIO_PRECHARGE_CH);
        //         gpio.clear(GPIO_DCDC_EN_PORT, GPIO_DCDC_EN_CH);
        //         is_actively_charging = false;
        //     }
        //     charged = (bms.input.highest_volt >= CHARGING_THRESHOLD) ? 1 : 0;
        // }
        // else {
        //     charged = 1;
        //     sendChargingCommands(false);
        //     gpio.clear(GPIO_AIR_NEG_PORT, GPIO_AIR_NEG_CH);
        //     gpio.clear(GPIO_AIR_POS_PORT, GPIO_AIR_POS_CH);
        //     gpio.clear(GPIO_PRECHARGE_PORT, GPIO_PRECHARGE_CH);
        //     gpio.clear(GPIO_DCDC_EN_PORT, GPIO_DCDC_EN_CH);
        //     is_actively_charging = false;
        // }
    
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

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1000));
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