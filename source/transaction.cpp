#include "transaction.h"

QueueHandle_t commandQueue = NULL;
SemaphoreHandle_t cbSemaphore;

bool checkPECS(uint8_t* rx, int len, int num) {
    int pos = 4;
    uint8_t pec[2];
    
    for(int i = 0; i < num; i++) {
        pec15_calc(len, &rx[pos], pec);
        pos+=len;
        if(rx[pos] != pec[0] || rx[pos+1] != pec[1]) {
            return false;
        }
        pos+=2;
    }

    return true;
}

// when spi finishes we give the semaphore back to the transaction
void bmsspicb() {
    BaseType_t taskWoken = 0;
    xSemaphoreGiveFromISR(cbSemaphore, &taskWoken);
}

// keeping queue and semaphore creation here should keep them from taking up
// space in the task stack, i think.
void transactionInit()
{
    // create a queue capable of containing 5 commands
    // TODO: check if null
    commandQueue = xQueueCreate( 5, sizeof(bmscommand_t));

    cbSemaphore = xSemaphoreCreateBinary();

    // Check if semaphore is NULL as this means it was not created
    if(cbSemaphore == NULL){
        // TODO: panic LED
        for(;;);
    }
}

void transaction( void *pvParameters )
{
    bmscommand_t receiveCommand;
    long lastMessage = xTaskGetTickCount();
    long time;

    spi::SPI& spi = spi::SPI::StaticClass();
    gpio::GPIO& gpio = gpio::GPIO::StaticClass();

    uint8_t start = 1; // 1 = do startup sequence

    for (;;)
    {

        xQueueReceive(commandQueue, (void*) &receiveCommand, portMAX_DELAY);

        // always do full wakeup on first time through
        time = start ? t_SLEEP + 1 : 
            (xTaskGetTickCount() - lastMessage);
        start = 0;
        
        //create spi buffers
        int length = bmsCommandSize(&receiveCommand);
        uint8_t *tx = (uint8_t*)(pvPortMalloc(length * sizeof(uint8_t)));
        uint8_t *rx = (uint8_t*)(pvPortMalloc(length * sizeof(uint8_t)));

        if(tick_us(time) > t_SLEEP)
        {
            //add reprograming sequence
        }
        if(tick_us(time) > t_IDLE)
        {
            // toggle the CS down and up once for each slave to wake
            for(int i = 0; i < SLAVE_COUNT; i++) {
                gpio.clear(ltccsport, ltccspin);
                //wait 400 microseconds
                // vTaskDelay(.4 / portTICK_PERIOD_MS);
                vTaskDelay(tick_us(400));
                gpio.set(ltccsport, ltccspin);
                vTaskDelay(tick_us(100));
            }
        }

        //add command into the buffer
        //if(receiveCommand.buildbuffer(&receiveCommand, tx))
        if(0)
        {
            // command decoding failed, maybe invalid command?
            // TODO: return to sender with error code
        }

        //TODO: Ensure that the semaphore is working properly

        //send spi messaage
        spi.mastertx(0, tx, rx, length);

        // wait to get semaphore back from spi (Currently waits infinitely)
        if(xSemaphoreTake(cbSemaphore, portMAX_DELAY) == pdTRUE)
        {

            lastMessage = xTaskGetTickCount();
            // spi finished
            // TODO: handle response, check pecs
            if(!checkPECS(rx, receiveCommand.size, receiveCommand.num)) {
                // PEC Check failed 
            } else {
                // return result to caller
                memcpy(receiveCommand.result, rx, length);
                // vPortFree the space for tx
            }
            
        } else {
            // spi timed out return NULL to sender
            // vPortFree the space for tx
        }

        vPortFree(tx);
        vPortFree(rx);
        xSemaphoreGive(receiveCommand.semaphore);
        
    }
}

// semaphore for pushCommand. created in main.cpp
extern SemaphoreHandle_t pushsemaphore;
// push given command to command queue 
void pushCommand( uint8_t *com, int length, int num, uint8_t *data, uint8_t *rx, int ticksToWait ) {
    // assemble the command
    bmscommand_t c;

    bmsCommandInit(&c, com, length, num, data, rx, pushsemaphore, NULL);
    // sends command to the command queue and returns the error or success code
    xQueueSend(commandQueue, &c, ticksToWait); //TODO: check for failure and return NULL

    // wait for callback and return its return
    xSemaphoreTake(pushsemaphore, portMAX_DELAY); //TODO: check for failure and return NULL
    
}


/* NONFUNCTIONAL
 * Creates and pushes a command to the command queue asynchronously
 *      @param com: the command id
 *      @param length: the length of the command data
 *      @param num: the number of chips that this command has data for
 *      @param data: all the chips command data (must remmain allocated untill the command is completed)
 *      @param ticksToWait: The number of ticks to wait before timing out or portMAX_DELAY for infinite block
 *      @param result: location for the return value of the command to be placed
 *
 *      @return Returns pdTRUE if the item was successfully posted or errQUEUE_FULL if the request times out.
 */
int sendCommandAsync( int com, int length, int num, uint8_t *data, int ticksToWait, uint8_t* result)
{
    /*
    // assemble the command
    bmscommand_t c;
    SemaphoreHandle_t xSemaphore = xSemaphoreCreateBinary(); // wont work needs to be allocated
    bmsCommandInit(&c, com, length, num, data, result, xSemaphore);
    // sends command to the command queue and returns the error or success code
    return xQueueSend(commandQueue, &c, ticksToWait); 
    */
   return 0;
}
