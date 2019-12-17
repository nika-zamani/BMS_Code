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
            ((xTaskGetTickCount() - lastMessage)/portTICK_PERIOD_MS);
        start = 0;
        
        //create spi buffers
        int length = bmsCommandSize(&receiveCommand);
        uint8_t *tx = (uint8_t*)(pvPortMalloc(length * sizeof(uint8_t)));
        uint8_t *rx = (uint8_t*)(pvPortMalloc(length * sizeof(uint8_t)));

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
                // vTaskDelay(.4 / portTICK_PERIOD_MS);
                vTaskDelay(tick_us(400));
                gpio.set(ltccsport, ltccspin);
                vTaskDelay(tick_us(100));
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
        spi.mastertx(0, tx, rx, length);

        // wait to get semaphore back from spi (Currently waits infinitely)
        if(xSemaphoreTake(cbSemaphore, portMAX_DELAY) == pdTRUE)
        {

            lastMessage = xTaskGetTickCount();
            // spi finnished
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

    bmsCommandInit(&c, com, length, num, data, rx, pushsemaphore);
    // sends command to the command queue and returns the error or success code
    xQueueSend(commandQueue, &c, ticksToWait); //TODO: check for failure and return NULL

    // wait for callback and return its return
    xSemaphoreTake(pushsemaphore, portMAX_DELAY); //TODO: check for failure and return NULL
    
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
 *      @return Returns the resultant data of the command, must be vPortFree'd once no longer needed
 */
uint8_t new_com[2];

void sendCommand( int com, int length, int num, uint8_t *data, uint8_t *rx, int ticksToWait )
{
    //uint8_t *new_com = (uint8_t*)(pvPortMalloc(2*sizeof(uint8_t)));
    new_com[0] = CCS[com][0];
    new_com[1] = CCS[com][1];
    pushCommand( new_com, length, num, data, rx, ticksToWait );
    //vPortFree(new_com);
}
/*
// call ADCV command like any other but with base 10 integer values for MD, DCP, and CH
uint8_t* sendCommandADCV( int md, int dcp, int ch, int length, int num, uint8_t *data, int ticksToWait ) {
    int com = ADCV;
    uint8_t *new_com = (uint8_t*)(pvPortMalloc(2*sizeof(uint8_t)));
    new_com[0] = CCS[com][0] | md>>1;
    new_com[1] = CCS[com][1] | (md%2 * 128);
    new_com[1] = new_com[1] | (dcp * 16);
    new_com[1] = new_com[1] | (ch); 
    uint8_t* ret = pushCommand(new_com, length, num, data, ticksToWait );
    vPortFree(new_com);
    return ret;
}

// call ADOW command like any other but with base 10 integer values for MD, PUP, DCP, and CH
uint8_t* sendCommandADOW( int md, int pup, int dcp, int ch, int length, int num, uint8_t *data, int ticksToWait ) {
    int com = ADOW;    
    uint8_t *new_com = (uint8_t*)(pvPortMalloc(2*sizeof(uint8_t)));
    new_com[0] = CCS[com][0] | md>>1;
    new_com[1] = CCS[com][1] | (md%2 * 128);
    new_com[1] = new_com[1] | (pup * 64);
    new_com[1] = new_com[1] | (dcp * 16);
    new_com[1] = new_com[1] | (ch); 
    uint8_t* ret = pushCommand(new_com, length, num, data, ticksToWait );
    vPortFree(new_com);
    return ret;
}

// call CVST command like any other but with base 10 integer values for MD, and ST
uint8_t* sendCommandCVST( int md, int st, int length, int num, uint8_t *data, int ticksToWait ) {
    int com = CVST;
    uint8_t *new_com = (uint8_t*)(pvPortMalloc(2*sizeof(uint8_t)));
    new_com[0] = CCS[com][0] | md>>1;
    new_com[1] = CCS[com][1] | (md%2 * 128);
    new_com[1] = new_com[1] | (st * 32);
    uint8_t* ret = pushCommand(new_com, length, num, data, ticksToWait );
    vPortFree(new_com);
    return ret;
}

// call ADOL command like any other but with base 10 integer values for MD, and DCP
uint8_t* sendCommandADOL( int md, int dcp, int length, int num, uint8_t *data, int ticksToWait ) {
    int com = ADOL;
    uint8_t *new_com = (uint8_t*)(pvPortMalloc(2*sizeof(uint8_t)));
    new_com[0] = CCS[com][0] | md>>1;
    new_com[1] = CCS[com][1] | (md%2 * 128);
    new_com[1] = new_com[1] | (dcp * 16);
    uint8_t* ret = pushCommand(new_com, length, num, data, ticksToWait );
    vPortFree(new_com);
    return ret;
}

// call ADAX command like any other but with base 10 integer values for MD, and CHG
uint8_t* sendCommandADAX( int md, int chg, int length, int num, uint8_t *data, int ticksToWait ) {
    int com = ADAX;
    uint8_t *new_com = (uint8_t*)(pvPortMalloc(2*sizeof(uint8_t)));
    new_com[0] = CCS[com][0] | md>>1;
    new_com[1] = CCS[com][1] | (md%2 * 128);
    new_com[1] = new_com[1] | (chg); 
    uint8_t* ret = pushCommand(new_com, length, num, data, ticksToWait );
    vPortFree(new_com);
    return ret;
}

// call ADAXD command like any other but with base 10 integer values for MD, and CHG
uint8_t* sendCommandADAXD( int md, int chg, int length, int num, uint8_t *data, int ticksToWait ) {
    int com = ADAXD;
    uint8_t *new_com = (uint8_t*)(pvPortMalloc(2*sizeof(uint8_t)));
    new_com[0] = CCS[com][0] | md>>1;
    new_com[1] = CCS[com][1] | (md%2 * 128);
    new_com[1] = new_com[1] | (chg); 
    uint8_t* ret = pushCommand(new_com, length, num, data, ticksToWait );
    vPortFree(new_com);
    return ret;
}

// call AXST command like any other but with base 10 integer values for MD, and ST
uint8_t* sendCommandAXST( int md, int st, int length, int num, uint8_t *data, int ticksToWait ) {
    int com = AXST;
    uint8_t *new_com = (uint8_t*)(pvPortMalloc(2*sizeof(uint8_t)));
    new_com[0] = CCS[com][0] | md>>1;
    new_com[1] = CCS[com][1] | (md%2 * 128);
    new_com[1] = new_com[1] | (st * 32);
    uint8_t* ret = pushCommand(new_com, length, num, data, ticksToWait );
    vPortFree(new_com);
    return ret;
}

// call ADSTAT command like any other but with base 10 integer values for MD, and CHST
uint8_t* sendCommandADSTAT( int md, int chst, int length, int num, uint8_t *data, int ticksToWait ) {
    int com = ADSTAT;
    uint8_t *new_com = (uint8_t*)(pvPortMalloc(2*sizeof(uint8_t)));
    new_com[0] = CCS[com][0] | md>>1;
    new_com[1] = CCS[com][1] | (md%2 * 128);
    new_com[1] = new_com[1] | (chst); 
    uint8_t* ret = pushCommand(new_com, length, num, data, ticksToWait );
    vPortFree(new_com);
    return ret;
}

// call ADSTATD command like any other but with base 10 integer values for MD, and CHST
uint8_t* sendCommandADSTATD( int md, int chst, int length, int num, uint8_t *data, int ticksToWait ) {
    int com = ADSTATD;
    uint8_t *new_com = (uint8_t*)(pvPortMalloc(2*sizeof(uint8_t)));
    new_com[0] = CCS[com][0] | md>>1;
    new_com[1] = CCS[com][1] | (md%2 * 128);
    new_com[1] = new_com[1] | (chst); 
    uint8_t* ret = pushCommand(new_com, length, num, data, ticksToWait );
    vPortFree(new_com);
    return ret;
}

// call STATST command like any other but with base 10 integer values for MD, and ST
uint8_t* sendCommandSTATST( int md, int st, int length, int num, uint8_t *data, int ticksToWait ) {
    int com = STATST;
    uint8_t *new_com = (uint8_t*)(pvPortMalloc(2*sizeof(uint8_t)));
    new_com[0] = CCS[com][0] | md>>1;
    new_com[1] = CCS[com][1] | (md%2 * 128);
    new_com[1] = new_com[1] | (st * 32);
    uint8_t* ret = pushCommand(new_com, length, num, data, ticksToWait );
    vPortFree(new_com);
    return ret;
}

// call ADCVAX command like any other but with base 10 integer values for MD, and DCP
uint8_t* sendCommandADCVAX( int md, int dcp, int length, int num, int ticksToWait ) {
    int com = ADCVAX;
    uint8_t *new_com = (uint8_t*)(pvPortMalloc(2*sizeof(uint8_t)));
    new_com[0] = CCS[com][0] | md>>1;
    new_com[1] = CCS[com][1] | (md%2 * 128);
    new_com[1] = new_com[1] | (dcp * 16);
    uint8_t* ret = pushCommand(new_com, length, num, NULL, ticksToWait );
    vPortFree(new_com);
    return ret;
}

// call ADCVSC command like any other but with base 10 integer values for MD, and DCP
uint8_t* sendCommandADCVSC( int md, int dcp, int length, int num, uint8_t *data, int ticksToWait ) {
    int com = ADCVSC;
    uint8_t *new_com = (uint8_t*)(pvPortMalloc(2*sizeof(uint8_t)));
    new_com[0] = CCS[com][0] | md>>1;
    new_com[1] = CCS[com][1] | (md%2 * 128);
    new_com[1] = new_com[1] | (dcp * 16);
    uint8_t* ret = pushCommand(new_com, length, num, data, ticksToWait );
    vPortFree(new_com);
    return ret;
}
*/

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

// when spi finishes we give the semaphore back to the transaction
void bmsspicb() {
    BaseType_t taskWoken = 0;
    xSemaphoreGiveFromISR(cbSemaphore, &taskWoken);
}

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
