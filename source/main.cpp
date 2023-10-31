#include "main.h"

using namespace BSP;

uint32_t utils[50]= {}; 
uint32_t idle_cnt = 0;
uint32_t tick_cnt = 0;
uint32_t i = 0;
uint32_t chip_util = 0;

void idle_task(void *) {
	for (;;) {
		idle_cnt += 1;
		vTaskDelay(1);
	}
}


BMS bms;


int main(void)
{
    prvSetupHardware();
    transactionInit();
    
    taskInit();
    xTaskCreate(idle_task, "basic_task,", STACK_SIZE,NULL, 1 ,NULL);


    gpio::GPIO::clear(ltccsport, ltccspin);

    vTaskStartScheduler();

    return 0;
}

extern "C"
{
    void vApplicationMallocFailedHook(void)
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
        for (;;)
            ;
    }

    void vApplicationIdleHook(void)
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

    void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName)
    {
        (void)pcTaskName;
        (void)pxTask;

        /* Run time stack overflow checking is performed if
         * configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
         * function is called if a stack overflow is detected. */
        taskDISABLE_INTERRUPTS();
        for (;;)
            ;
    }

    void vApplicationTickHook(void)
    {
        if (tick_cnt >= 10000) {
			uint32_t chip_util = (idle_cnt * 100)/ tick_cnt;
			
			utils[i] = chip_util;

			if(i < 50) {
				i += 1; }
			else {
				i = 0;
			}
			tick_cnt = 0; //reset tick
			idle_cnt = 0; }//reset idle tick  
		else {
			tick_cnt +=1 ; }//count tick 
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
        configASSERT(memcmp((void *)_pvHeapStart, ucExpectedInterruptStackValues, sizeof(ucExpectedInterruptStackValues)) == 0U);
#endif /* mainCHECK_INTERRUPT_STACK */
    }
}
