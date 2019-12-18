#include "testroutines.h"

uint8_t RETURN_DATA[6];

void commandRDCFGA( void *pvParameters )
{
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = tick_ms(1000);

    // Initialise the xLastWakeTime variable with the current time.
    xLastWakeTime = xTaskGetTickCount();

    for (;;)
    {
        int error = pushCommand(RDCFGA, SLAVE_COUNT, RETURN_DATA);

        vTaskDelayUntil( &xLastWakeTime, xFrequency );
    }
}

void commandWRCFGA( void *pvParameters )
{
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = tick_ms(1000);

    // Initialise the xLastWakeTime variable with the current time.
    xLastWakeTime = xTaskGetTickCount();

    for (;;)
    {
        uint8_t data[SLAVE_COUNT * 6] = {
            0x0, 0x0, 0x0, 0x0, 0x1, 0x0
        };

        volatile int error = pushCommand(WRCFGA, SLAVE_COUNT, data);

        vTaskDelayUntil( &xLastWakeTime, xFrequency );
    }
}

void commandRW( void *pvParameters )
{
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = tick_ms(1000);

    // Initialise the xLastWakeTime variable with the current time.
    xLastWakeTime = xTaskGetTickCount();

    for (;;)
    {
        uint8_t data[SLAVE_COUNT * 6] = {
            0x0, 0x0, 0x0, 0x0, 0x1, 0x0
        };

        volatile int error = pushCommand(WRCFGA, SLAVE_COUNT, data);

        vTaskDelayUntil( &xLastWakeTime, xFrequency/2 );

        error = pushCommand(RDCFGA, SLAVE_COUNT, RETURN_DATA);

        vTaskDelayUntil( &xLastWakeTime, xFrequency/2 );

    }
}
