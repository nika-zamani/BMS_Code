#include "transaction.h"

QueueHandle_t commandQueue = NULL;
SemaphoreHandle_t cbSemaphore;

void transaction( void *pvParameters )
{
    bmscommand_t receiveCommand;
    long lastMessage = xTaskGetTickCount();
    long time;

    spi::SPI& spi = spi::SPI::StaticClass();
    gpio::GPIO& gpio = gpio::GPIO::StaticClass();


    for (;;)
    {
        /* demo task coode */
        xQueueReceive(commandQueue, (void*) &receiveCommand, portMAX_DELAY);
        time = (xTaskGetTickCount() - lastMessage)/portTICK_PERIOD_MS;
        
        //create spi buffers
        int length = bmsCommandSize(&receiveCommand);
        uint8_t *tx = (uint8_t*)(malloc(length * sizeof(uint8_t)));
        uint8_t *rx = (uint8_t*)(malloc(length * sizeof(uint8_t)));

        if(time > t_SLEEP)
        {
            //add reprograming sequence
        }
        if(time > t_IDLE)
        {
            //add wakeup sequence

            // toggle the CS down and up once for each slave
            for(int i = 0; i < SLAVE_COUNT; i++) {
                gpio.clear(ltccsport, ltccspin);
                //wait 400 microseconds
                vTaskDelay(.4 / portTICK_PERIOD_MS);
                gpio.set(ltccsport, ltccspin);
                vTaskDelay(.1 / portTICK_PERIOD_MS);
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

        // free the space for tx
        free(tx);

        // wait to get semaphore back from spi (Currently waits infinitely)
        if(xSemaphoreTake(cbSemaphore, portMAX_DELAY) == pdTRUE)
        {
            // spi finnished
            // handle response
        } else {
            // spi timed out
        }
        
        // TODO check for memory leaks
        receiveCommand.result = rx;
        xSemaphoreGive(receiveCommand.semaphore);
    }
}

/*
 * Creates and pushes a command to the command queue synchronously only returning once the command has been executed
 *      @param com: the command id
 *      @param length: the length of the command data
 *      @param num: the number of chips that this command has data for
 *      @param data: all the chips command data (must remmain allocated untill the command is completed)
 *      @param ticksToWait: The number of ticks to wait before timing out or portMAX_DELAY for infinite block
 *
 *      @return Returns the resultant data of the command
 */
uint8_t* sendCommand( int com, int length, int num, uint8_t **data, int ticksToWait )
{
    uint8_t* result;
    SemaphoreHandle_t xSemaphore = xSemaphoreCreateBinary();
    // assemble the command
    bmscommand_t c;
    bmsCommandInit(&c, com, length, num, data, result, xSemaphore);
    // sends command to the command queue and returns the error or success code
    xQueueSend(commandQueue, &c, ticksToWait);

    // wait for callback and return its return
    xSemaphoreTake(xSemaphore, portMAX_DELAY);
    return result;
}

/*
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
int sendCommandAsync( int com, int length, int num, uint8_t **data, int ticksToWait, uint8_t* result)
{
    // assemble the command
    bmscommand_t c;
    SemaphoreHandle_t xSemaphore = xSemaphoreCreateBinary(); // wont work needs to be allocated
    bmsCommandInit(&c, com, length, num, data, result, xSemaphore);
    // sends command to the command queue and returns the error or success code
    return xQueueSend(commandQueue, &c, ticksToWait); 
}

// when spi finishes we give the semaphore back to the transaction
void bmsspicb() {
    BaseType_t taskWoken = 0;
    xSemaphoreGiveFromISR(cbSemaphore, &taskWoken);
}

void transactionInit()
{
    // create a queue capable of containing 5 commands
    commandQueue = xQueueCreate( 5, sizeof(bmscommand_t));

    gpio::GPIO::ConstructStatic();

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