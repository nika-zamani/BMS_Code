/* Standard includes */
#include "string.h"

/* FreeRTOS includes */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Hardware specific includes */

#include "MKE18F16.h"
#include "clock_config.h"
#include "pin_mux.h"

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

// Array contains command codes for all commands neccecary, position in this array will depend
//      on a mapped value from the command name in bms.h
const unsigned char CCS[][] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00}, //RDSPSB*
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x01}  //STSCTRL
}

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
    gpio::GPIO& gpio = gpio::GPIO::StaticClass();

    TickType_t xLastWakeTime;
    const TickType_t xFrequency = 300 / portTICK_PERIOD_MS;

    // Initialise the xLastWakeTime variable with the current time.
    xLastWakeTime = xTaskGetTickCount();

    int sendValue = 1;
    for (;;)
    {
        /* demo task coode */
        xQueueSend(xQueue, (void*) &sendValue, portMAX_DELAY);
        vTaskDelayUntil( &xLastWakeTime, xFrequency );
    }
}

void transaction( void *pvParameters )
{
    int receiveBuff;
    long lastMessage = xTaskGetTickCount();
    long time;

    spi::SPI& spi = spi::SPI::StaticClass();

    for (;;)
    {
        /* demo task coode */
        xQueueReceive(xQueue, (void*) &receiveBuff, portMAX_DELAY);
        time = (xTaskGetTickCount() - lastMessage)/portTICK_PERIOD_MS 
        
        //create spi buffer
        uint8_t tx[8];
        uint8_t rx[8];

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
        if(!decodeCommand(command, &tx))
        {
            // command decoding failed, maybe invalid command?
            // return to sender with error code
        }

        // Give the semaphore to spi (Don't want to do this unless spi can take the semaphore
        //      otherwise we may give the semaphore to ourselves)
        // xSemaphoreGive(cbSemaphore);
        
        //send spi messaage
        spi.mastertx(0, tx, rx, 8);
        // wait to get semaphore back from spi (Currently waits infinitely)
        if(xSemaphoreTake(cbSemaphore, portMAX_DELAY) == pdTRUE)
        {
            // spi finnished
            // handle response
        } else {
            // spi timed out
        }
        
        // send semaphore to sender

    }
}

// maps an integer command id to its corrosponding command and
//      loads said commands SPI command into the spi array.
//  command: The integer command id
//  tx: A pointer to the spi array
//  
//  return: an integer error code or 1 if successfull
int decodeCommand(int command, uint8_t **tx)
{
    // TODO: error checking of command number
    unsigned char cc[11] = CCS[command];
    // create the tx out of the cc

    return 1;
}

// when spi finishes we give the semaphore back to the transaction
void bmsspicb() {
    xSemaphoreGive(cbSemaphore);
}

int main( void ) {
    // setup the microcontroller hardware for the demo 
    prvSetupHardware();

    // create task
    //TaskHandle_t xHandle = NULL;

    // create a queue capable of containing 5 integers
    xQueue = xQueueCreate( 5, sizeof(int));
    //TODO: check if xQueue is NULL as this means it was not created

    xTaskCreate(demoReceive, "demo", STACK_SIZE, NULL, TASK_PRIORITY, NULL );
    xTaskCreate(demoDelaySend, "demoUntil", STACK_SIZE, NULL, TASK_PRIORITY, NULL );

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

    spi::SPI::masterConfig mconf;
    mconf.baudRate = 500000U;
    mconf.csport = ltccsport;
    mconf.cspin = ltccspin;
    spi.initMaster(0, &mconf);

    cbSemaphore = xSemaphoreCreateBinary();
    // TODO: Check if semaphore is NULL as this means it was not created
}

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

