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
            // toggle the CS down and up once for each slave to wake
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
            // TODO: return to sender with error code
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
            // TODO: handle response, check pecs
            
            // return result to caller
            receiveCommand.result = rx;
            xSemaphoreGive(receiveCommand.semaphore);
        } else {
            // spi timed out return null to sender
            free(rx);
            receiveCommand.result = NULL;
            xSemaphoreGive(receiveCommand.semaphore);
        }
        
        // TODO: check for memory leaks
    }
}

/*
 * Creates and pushes a command to the command queue synchronously only returning once the command has been executed.
 *      @param com: the command id
 *      @param length: the length of the command data
 *      @param num: the number of chips that this command has data for
 *      @param data: all the chips command data (must remmain allocated untill the command is completed)
 *      @param ticksToWait: The number of ticks to wait before timing out or portMAX_DELAY for infinite block
 *                              evenly divided between sending to the command queue and waiting for command to return
 *
 *      @return Returns the resultant data of the command, must be free'd once no longer needed
 */
uint8_t* sendCommand( int com, int length, int num, uint8_t **data, int ticksToWait )
{
    uint8_t* result;
    SemaphoreHandle_t xSemaphore = xSemaphoreCreateBinary();
    // assemble the command
    bmscommand_t c;
    bmsCommandInit(&c, CCS[com], length, num, data, result, xSemaphore);
    // sends command to the command queue and returns the error or success code
    xQueueSend(commandQueue, &c, ticksToWait/2); //TODO: check for failure and return NULL

    // wait for callback and return its return
    xSemaphoreTake(xSemaphore, ticksToWait/2); //TODO: check for failure and return NULL
    
    return result; // can be NULL if add to queue fails, command takes too long to return, or spi never returned in transaction
}

// call ADCV command like any other but with base 10 integer values for MD, DCP, and CH
uint8_t* sendCommandADCV( int md, int dcp, int ch, int length, int num, uint8_t **data, int ticksToWait ) {
    int com = ADCV;
    uint8_t *new_com = malloc(2*sizeof(uint8_t));
    new_com[0] = CCS[com][0] | md>>1;
    new_com[1] = CCS[com][1] | (md%2 * 128);
    new_com[1] = new_com[1] | (dcp * 16);
    new_com[1] = new_com[1] | (ch); 
    uint8_t* ret = sendCommand( new_com, length, num, data, ticksToWait );
    free(new_com);
    return ret;
}

// call ADOW command like any other but with base 10 integer values for MD, PUP, DCP, and CH
uint8_t* sendCommandADOW( int md, int pup, int dcp, int ch, int length, int num, uint8_t **data, int ticksToWait ) {
    int com = ADOW;    
    uint8_t *new_com = malloc(2*sizeof(uint8_t));
    new_com[0] = CCS[com][0] | md>>1;
    new_com[1] = CCS[com][1] | (md%2 * 128);
    new_com[1] = new_com[1] | (pup * 64);
    new_com[1] = new_com[1] | (dcp * 16);
    new_com[1] = new_com[1] | (ch); 
    uint8_t* ret = sendCommand( new_com, length, num, data, ticksToWait );
    free(new_com);
    return ret;
}

// call CVST command like any other but with base 10 integer values for MD, and ST
uint8_t* sendCommandCVST( int md, int st, int length, int num, uint8_t **data, int ticksToWait ) {
    int com = CVST;
    uint8_t *new_com = malloc(2*sizeof(uint8_t));
    new_com[0] = CCS[com][0] | md>>1;
    new_com[1] = CCS[com][1] | (md%2 * 128);
    new_com[1] = new_com[1] | (st * 32);
    uint8_t* ret = sendCommand( new_com, length, num, data, ticksToWait );
    free(new_com);
    return ret;
}

// call ADOL command like any other but with base 10 integer values for MD, and DCP
uint8_t* sendCommandADOL( int md, int dcp, int length, int num, uint8_t **data, int ticksToWait ) {
    int com = ADOL;
    uint8_t *new_com = malloc(2*sizeof(uint8_t));
    new_com[0] = CCS[com][0] | md>>1;
    new_com[1] = CCS[com][1] | (md%2 * 128);
    new_com[1] = new_com[1] | (dcp * 16);
    uint8_t* ret = sendCommand( new_com, length, num, data, ticksToWait );
    free(new_com);
    return ret;
}

// call ADAX command like any other but with base 10 integer values for MD, and CHG
uint8_t* sendCommandADAX( int md, int chg, int length, int num, uint8_t **data, int ticksToWait ) {
    int com = ADAX;
    uint8_t *new_com = malloc(2*sizeof(uint8_t));
    new_com[0] = CCS[com][0] | md>>1;
    new_com[1] = CCS[com][1] | (md%2 * 128);
    new_com[1] = new_com[1] | (chg); 
    uint8_t* ret = sendCommand( new_com, length, num, data, ticksToWait );
    free(new_com);
    return ret;
}

// call ADAXD command like any other but with base 10 integer values for MD, and CHG
uint8_t* sendCommandADAXD( int md, int chg, int length, int num, uint8_t **data, int ticksToWait ) {
    int com = ADAXD;
    uint8_t *new_com = malloc(2*sizeof(uint8_t));
    new_com[0] = CCS[com][0] | md>>1;
    new_com[1] = CCS[com][1] | (md%2 * 128);
    new_com[1] = new_com[1] | (chg); 
    uint8_t* ret = sendCommand( new_com, length, num, data, ticksToWait );
    free(new_com);
    return ret;
}

// call AXST command like any other but with base 10 integer values for MD, and ST
uint8_t* sendCommandAXST( int md, int st, int length, int num, uint8_t **data, int ticksToWait ) {
    int com = AXST;
    uint8_t *new_com = malloc(2*sizeof(uint8_t));
    new_com[0] = CCS[com][0] | md>>1;
    new_com[1] = CCS[com][1] | (md%2 * 128);
    new_com[1] = new_com[1] | (st * 32);
    uint8_t* ret = sendCommand( new_com, length, num, data, ticksToWait );
    free(new_com);
    return ret;
}

// call ADSTAT command like any other but with base 10 integer values for MD, and CHST
uint8_t* sendCommandADSTAT( int md, int chst, int length, int num, uint8_t **data, int ticksToWait ) {
    int com = ADSTAT;
    uint8_t *new_com = malloc(2*sizeof(uint8_t));
    new_com[0] = CCS[com][0] | md>>1;
    new_com[1] = CCS[com][1] | (md%2 * 128);
    new_com[1] = new_com[1] | (chst); 
    uint8_t* ret = sendCommand( new_com, length, num, data, ticksToWait );
    free(new_com);
    return ret;
}

// call ADSTATD command like any other but with base 10 integer values for MD, and CHST
uint8_t* sendCommandADSTATD( int md, int chst, int length, int num, uint8_t **data, int ticksToWait ) {
    int com = ADSTATD;
    uint8_t *new_com = malloc(2*sizeof(uint8_t));
    new_com[0] = CCS[com][0] | md>>1;
    new_com[1] = CCS[com][1] | (md%2 * 128);
    new_com[1] = new_com[1] | (chst); 
    uint8_t* ret = sendCommand( new_com, length, num, data, ticksToWait );
    free(new_com);
    return ret;
}

// call STATST command like any other but with base 10 integer values for MD, and ST
uint8_t* sendCommandSTATST( int md, int st, int length, int num, uint8_t **data, int ticksToWait ) {
    int com = STATST;
    uint8_t *new_com = malloc(2*sizeof(uint8_t));
    new_com[0] = CCS[com][0] | md>>1;
    new_com[1] = CCS[com][1] | (md%2 * 128);
    new_com[1] = new_com[1] | (st * 32);
    uint8_t* ret = sendCommand( new_com, length, num, data, ticksToWait );
    free(new_com);
    return ret;
}

// call ADCVAX command like any other but with base 10 integer values for MD, and DCP
uint8_t* sendCommandADCVAX( int md, int dcp, int length, int num, uint8_t **data, int ticksToWait ) {
    int com = ADCVAX;
    uint8_t *new_com = malloc(2*sizeof(uint8_t));
    new_com[0] = CCS[com][0] | md>>1;
    new_com[1] = CCS[com][1] | (md%2 * 128);
    new_com[1] = new_com[1] | (dcp * 16);
    uint8_t* ret = sendCommand( new_com, length, num, data, ticksToWait );
    free(new_com);
    return ret;
}

// call ADCVSC command like any other but with base 10 integer values for MD, and DCP
uint8_t* sendCommandADCVSC( int md, int dcp, int length, int num, uint8_t **data, int ticksToWait ) {
    int com = ADCVSC;
    uint8_t *new_com = malloc(2*sizeof(uint8_t));
    new_com[0] = CCS[com][0] | md>>1;
    new_com[1] = CCS[com][1] | (md%2 * 128);
    new_com[1] = new_com[1] | (dcp * 16);
    uint8_t* ret = sendCommand( new_com, length, num, data, ticksToWait );
    free(new_com);
    return ret;
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
int sendCommandAsync( int com, int length, int num, uint8_t **data, int ticksToWait, uint8_t* result)
{
    return 0;
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