/* Standard includes */
#include "string.h"
#include <vector>
#include <array>

/* FreeRTOS includes */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* Hardware specific includes */

#include "MKE18F16.h"
#include "clock_config.h"
#include "pin_mux.h"

#include "bmsCommand.h"

#include "gpio.h"
#include "spi.h"

using namespace BSP;
/* The configCHECK_FOR_STACK_OVERFLOW setting in FreeRTOSConifg can be used to
 * check task stacks for overflows.  It does not however check the stack used by
 * interrupts.  This demo has a simple addition that will also check the stack used
 * by interrupts if mainCHECK_INTERRUPT_STACK is set to 1.  Note that this check is
 * only performed from the tick hook function (which runs in an interrupt context).
 * It is a good debugging aid - but won't catch interrupt stack problems until the
 * tick interrupt next executes. */
#define mainCHECK_INTERRUPT_STACK 0
#if mainCHECK_INTERRUPT_STACK == 1
const unsigned char ucExpectedInterruptStackValues[] = { 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC };
#endif

#define STACK_SIZE 100
#define TASK_PRIORITY 1

#define SLAVE_COUNT 10

#define t_SLEEP 10000 // find this
#define t_IDLE 1000 // find this

/*
 * Perform any hardware specific setup in this function
 */
static void prvSetupHardware(void);

QueueHandle_t commandQueue = NULL;
SemaphoreHandle_t cbSemaphore;

void timeout( void *pvParameters )
{

}

void commandSend( void *pvParameters )
{
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = 300 / portTICK_PERIOD_MS;

    // Initialise the xLastWakeTime variable with the current time.
    xLastWakeTime = xTaskGetTickCount();

    int sendValue = 1;
    for (;;)
    {
        /* demo task coode */
        xQueueSend(commandQueue, (void*) &sendValue, portMAX_DELAY);
        vTaskDelayUntil( &xLastWakeTime, xFrequency );
    }
}

/*
 * Creates and pushes a command to the command queue   
 *      @param com: the command id
 *      @param length: the length of the command data
 *      @param num: the number of chips that this command has data for
 *      @param data: all the chips command data (must remmain allocated untill the command is completed)
 *      @param ticksToWait: The number of ticks to wait before timing out or portMAX_DELAY for infinite block
 *
 *      @return Returns pdTRUE if the item was successfully posted or errQUEUE_FULL if the request times out.
 */
int pushTransaction( int com, int length, int num, uint8_t **data, int ticksToWait )
{
    // assemble the command
    bmscommand_t *c;
    bmsCommandInit(c, com, length, num, data);
    // sends command to the command queue and returns the error or success code
    return xQueueSend(commandQueue, c, ticksToWait); 
}

void transaction( void *pvParameters )
{
    bmscommand_t receiveCommand;
    long lastMessage = xTaskGetTickCount();
    long time;

    spi::SPI& spi = spi::SPI::StaticClass();

    for (;;)
    {
        /* demo task coode */
        xQueueReceive(commandQueue, (void*) &receiveCommand, portMAX_DELAY);
        time = (xTaskGetTickCount() - lastMessage)/portTICK_PERIOD_MS;
        
        //create spi buffers
        int length = bmsCommandSize(&receiveCommand);
        uint8_t tx[length];
        uint8_t rx[length];

        if(time > t_SLEEP)
        {
            //add reprograming sequence
        }
        if(time > t_IDLE)
        {
            //add wakeup sequence

            // toggle the CS down and up once for each slave
            for(int i = 0; i < SLAVE_COUNT; i++) {
            }
        }

        //add command into the buffer
        if(!buildCommandBuffer(&receiveCommand, tx))
        {
            // command decoding failed, maybe invalid command?
            // return to sender with error code
        }

        //TODO: Ensure that the semaphore is working properly
        
        //send spi messaage
        spi.mastertx(0, &tx[0], &rx[0], length);
        // wait to get semaphore back from spi (Currently waits infinitely)
        if(xSemaphoreTake(cbSemaphore, portMAX_DELAY) == pdTRUE)
        {
            // spi finnished
            // handle response
        } else {
            // spi timed out
        }
        
        //TODO: figure out a way to return this data to the sender

    }
}

// when spi finishes we give the semaphore back to the transaction
void bmsspicb() {
    xSemaphoreGive(cbSemaphore);
}

int main( void ) {
    // setup the microcontroller hardware for the demo 
    prvSetupHardware();

    // create a queue capable of containing 5 commands
    commandQueue = xQueueCreate( 5, sizeof(bmscommand_t));
    //TODO: check if commandQueue is NULL as this means it was not created

    xTaskCreate(transaction, "transaction", STACK_SIZE, NULL, TASK_PRIORITY, NULL );
    xTaskCreate(commandSend, "commandSend", STACK_SIZE, NULL, TASK_PRIORITY, NULL );

    // Start the rtos scheduler, this function should never return as the execution context is changed to
    //      the task.

    vTaskStartScheduler();

    // shouldn't get here!
    return 0;
}


static void prvSetupHardware( void ) {
    // perform all hardare specific setup here
    BOARD_InitBootClocks();
    BOARD_InitBootPins();

    gpio::GPIO::ConstructStatic();
}

void spiInit()
{
    spi::spi_config conf;
    conf.callbacks[0] = bmsspicb;
    spi::SPI::ConstructStatic(&conf);
    spi::SPI& spi = spi::SPI::StaticClass();

    /*spi::SPI::masterConfig mconf;
    mconf.baudRate = 500000U;
    mconf.csport = ltccsport;
    mconf.cspin = ltccspin;
    spi.initMaster(0, &mconf);*/

    cbSemaphore = xSemaphoreCreateBinary();
    // TODO: Check if semaphore is NULL as this means it was not created
}

/************************************************************************************/
/*************************   RTOS CODE BEYOND THIS POINT   **************************/
/************************************************************************************/

extern "C" {
    void vApplicationMallocFailedHook( void )
    {
        /* vApplicationMallocFailedHook() will only be called if
         * configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
         * function that will get called if a call to pvPortMalloc() fails.
         * pvPortMalloc() is called internally by the kernel whenever a task, queue,
         * timer or semaphore is created.  It is also called by various parts of the
         * demo application.  If heap_1.c or heap_2.c are used, then the size of the
         * heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
         * FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
         * to query the size of free heap space that remains (although it does not
         * provide information on how the remaining heap might be fragmented). */
        taskDISABLE_INTERRUPTS();
        for( ;; );
    }

    void vApplicationIdleHook( void )
    {
        /* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
         * to 1 in FreeRTOSConfig.h.  It will be called on each iteration of the idle
         * task.  It is essential that code added to this hook function never attempts
         * to block in any way (for example, call xQueueReceive() with a block time
         * specified, or call vTaskDelay()).  If the application makes use of the
         * vTaskDelete() API function (as this demo application does) then it is also
         * important that vApplicationIdleHook() is permitted to return to its calling
         * function, because it is the responsibility of the idle task to clean up
         * memory allocated by the kernel to any task that has since been deleted. */
    }

    void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
    {
        ( void ) pcTaskName;
        ( void ) pxTask;

        /* Run time stack overflow checking is performed if
         * configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
         * function is called if a stack overflow is detected. */
        taskDISABLE_INTERRUPTS();
        for( ;; );
    }

    void vApplicationTickHook( void )
    {
#if mainCHECK_INTERRUPT_STACK == 1
        extern unsigned long _pvHeapStart[];

        /* This function will be called by each tick interrupt if
         * configUSE_TICK_HOOK is set to 1 in FreeRTOSConfig.h.  User code can be
         * added here, but the tick hook is called from an interrupt context, so
         * code must not attempt to block, and only the interrupt safe FreeRTOS API
         * functions can be used (those that end in FromISR()). */

        /* Manually check the last few bytes of the interrupt stack to check they
         * have not been overwritten.  Note - the task stacks are automatically
         * checked for overflow if configCHECK_FOR_STACK_OVERFLOW is set to 1 or 2
         * in FreeRTOSConifg.h, but the interrupt stack is not. */
        configASSERT( memcmp( ( void * ) _pvHeapStart, ucExpectedInterruptStackValues, sizeof( ucExpectedInterruptStackValues ) ) == 0U );
#endif /* mainCHECK_INTERRUPT_STACK */
    }
}

