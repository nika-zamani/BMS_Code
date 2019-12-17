#include "commandSend.h"

uint8_t TEST_DATA[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
uint8_t RETURN_DATA[12];

SemaphoreHandle_t pushsemaphore;

void commandSend( void *pvParameters )
{
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = tick_ms(1000);

    // Initialise the xLastWakeTime variable with the current time.
    xLastWakeTime = xTaskGetTickCount();

    pushsemaphore = xSemaphoreCreateBinary();

    int sendValue = 1;
    for (;;)
    {
        /* demo task coode */
        uint8_t data[NUM_CHIPS * DATA_LENGTH] = {
            0x0, 0x0, 0x0, 0x0, 0x1, 0x0
        };
        /*rx = sendCommand(WRCFGA, DATA_LENGTH, NUM_CHIPS, data, portMAX_DELAY);
        if (rx != NULL ) { 
            memcpy(RETURN_DATA, rx, 12);
            vPortFree(rx);  // DONT FORGET THIS FREE RETURN DATA IS ALLOCATED WHILE RECEIVING THE COMMAND
        } else {
           bool error = true; // error in command
        }*/

        sendCommand(RDCFGA, DATA_LENGTH, NUM_CHIPS, NULL, RETURN_DATA, portMAX_DELAY);

        vTaskDelayUntil( &xLastWakeTime, xFrequency );
    }
}

