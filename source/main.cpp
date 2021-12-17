#include "main.h"

using namespace BSP;

void timeout( void *pvParameters )
{

}

extern void bmsspicb();

static void prvSetupHardware( void ) {

    // perform all hardare specific setup here
    BOARD_InitBootClocks();
    BOARD_InitBootPins();

    gpio::GPIO::ConstructStatic();

    spi::spi_config conf;
    conf.callbacks[0] = bmsspicb;
    spi::SPI::ConstructStatic(&conf);
    spi::SPI& spi = spi::SPI::StaticClass();

    spi::SPI::masterConfig mconf;
    // parameterized in hardware.h
    mconf.baudRate = ltcbaud/10;
    mconf.csport = ltccsport;
    mconf.cspin = ltccspin;
    spi.initMaster(0, &mconf);

}


int main( void ) {
    prvSetupHardware(); //MKELib hw setup
    transactionInit();

    // reduce priority for lpspi interrupt
    // interrupt lower than freeRTOS so that it works :)
    NVIC->IP[26] |= 6 << 4;

    //TODO: check if commandQueue is NULL as this means it was not created
    //TODO: check for memory leaks

    xTaskCreate(transaction, "transaction", STACK_SIZE, NULL, configMAX_PRIORITIES-1, NULL );
    xTaskCreate(monitorBMSHealth, "monitorbmshlth", STACK_SIZE, NULL, 1, NULL );

    // Start the rtos scheduler, this function should never return as the
    // execution context is changed to the task.

    vTaskStartScheduler();

    // shouldn't get here!
    return 0;
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
         * to query the size of vPortFree heap space that remains (although it does not
         * provide information on how the remaining heap might be fragmented). */
        taskDISABLE_INTERRUPTS();
        for( ;; );
    }

    void vApplicationIdleHook( void )
    {
        // measure clock cycles when its not doing other stuff
        
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

