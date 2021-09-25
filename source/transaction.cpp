#include "transaction.h"
#include "StateMachine.h"

using namespace BSP;

static QueueHandle_t commandQueue = NULL;
static SemaphoreHandle_t cbSemaphore, pushSemaphore;

// Check to make sure received PECs are correct.
// Returns 0 on success, >0 on failure
static int checkPECS(uint8_t* rx, int len, int num) {
    int pos = 4;
    uint8_t pec[2];
    
    for(int i = 0; i < num; i++) {
        pec15_calc(6, rx+pos, pec);
        pos+=6;
        if(rx[pos] != pec[0] || rx[pos+1] != pec[1]) {
            return 1;
        }
        pos+=2;
    }

    return 0;
}

// when spi finishes we give the semaphore back to the transaction
void bmsspicb() {
    BaseType_t taskWoken = 0; // TODO what does this do?
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
    pushSemaphore = xSemaphoreCreateBinary();

    // Check if semaphore is NULL as this means it was not created
    if(cbSemaphore == NULL || pushSemaphore == NULL){
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
        int length = receiveCommand.len;
        uint8_t *tx = (uint8_t*)(pvPortMalloc(length * sizeof(uint8_t)));
        uint8_t *rx = (uint8_t*)(pvPortMalloc(length * sizeof(uint8_t)));

        //add command into the buffer
        if(receiveCommand.c.combb(&receiveCommand, tx))
        {
            // buffer assembly failed
            // TODO: return to sender with error code
        }

        if(time > tick_us(t_SLEEP))
        {
            //add reprograming sequence
        }
        if(time > tick_us(t_IDLE))
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


        //TODO: Ensure that the semaphore is working properly

        //send spi messaage
        spi.mastertx(0, tx, rx, length);

        // wait to get semaphore back from spi (Currently waits infinitely)
        if(xSemaphoreTake(cbSemaphore, portMAX_DELAY) == pdTRUE)
        {

            // spi finished
            lastMessage = xTaskGetTickCount();

            // somewhere here call state machine to keep track of rx from spi
            // TODO: find a way to pass through which bms board of 5
            StateMachine::setSPIRX(0, 0, rx);
            StateMachine::setSPIRX(0, 1, rx);

            if(receiveCommand.c.combb == combbRx){
                if(checkPECS(rx, length, receiveCommand.num)){  // check error case?
                    *(receiveCommand.error) = 1;
                } else {
                    for(int i = 0; i < receiveCommand.num; i++){
                        memcpy(receiveCommand.data+(i*6), rx+4+(i*8), 6);
                    }
                }
            } 
            
        } else {
            // spi timed out return NULL to sender
            // TODO: timeout handling
        }

        vPortFree(tx);
        vPortFree(rx);
        xSemaphoreGive(pushSemaphore);
        
    }
}

// push given command to command queue 
uint8_t pushCommand(uint8_t comn, int num, uint8_t *data, 
        uint8_t arg1, uint8_t arg2, uint8_t arg3, uint8_t arg4)
{
    // assemble the command
    bmscommand_t c;

    bmscom_t com = bmscom[comn];

    switch(comn){

        case ADCV: // arg1 = md, arg2 = dcp, arg3 = ch
            com.code[0] |= (arg1&0b10)>>1;
            com.code[1] |= (arg1&0b1)<<7
                | (arg2&0b1)<<4
                | (arg3&0b111)<<0;
            break;
        case ADOW: // arg1 = md, arg2 = pup, arg3 = dcp, arg4 = ch
            com.code[0] |= (arg1&0b10)>>1;
            com.code[1] |= (arg1&0b1)<<7
                | (arg2&0b1)<<6
                | (arg3&0b1)<<4
                | (arg4&0b111)<<0;
            break;
        case CVST: // arg1 = md, arg2 = st
            com.code[0] |= (arg1&0b10)>>1;
            com.code[1] |= (arg1&0b1)<<7
                | (arg2&0b11)<<6;
            break;
        case ADOL: // arg1 = md, arg2 = dcp
            com.code[0] |= (arg1&0b10)>>1;
            com.code[1] |= (arg1&0b1)<<7
                | (arg2&0b1)<<4;
            break;
        case ADAX: // arg1 = md, arg2 = chg
            com.code[0] |= (arg1&0b10)>>1;
            com.code[1] |= (arg1&0b1)<<7
                | (arg2&0b111)<<0;
            break;
        case ADAXD: // arg1 = md, arg2 = chg
            com.code[0] |= (arg1&0b10)>>1;
            com.code[1] |= (arg1&0b1)<<7
                | (arg2&0b111)<<0;
            break;
        case AXST: // arg1 = md, arg2 = st
            com.code[0] |= (arg1&0b10)>>1;
            com.code[1] |= (arg1&0b1)<<7
                | (arg2&0b11)<<6;
            break;
        case ADSTAT: // arg1 = md, arg2 = chst
            com.code[0] |= (arg1&0b10)>>1;
            com.code[1] |= (arg1&0b1)<<7
                | (arg2&0b11)<<6;
            break;
        case ADSTATD: // arg1 = md, arg2 = chst
            com.code[0] |= (arg1&0b10)>>1;
            com.code[1] |= (arg1&0b1)<<7
                | (arg2&0b11)<<6;
            break;
        case STATST: // arg1 = md, arg2 = st
            com.code[0] |= (arg1&0b10)>>1;
            com.code[1] |= (arg1&0b1)<<7
                | (arg2&0b11)<<6;
            break;
        case ADCVAX: // arg1 = md, arg2 = dcp
            com.code[0] |= (arg1&0b10)>>1;
            com.code[1] |= (arg1&0b1)<<7
                | (arg2&0b1)<<4;
            break;
        case ADCVSC: // arg1 = md, arg2 = dcp
            com.code[0] |= (arg1&0b10)>>1;
            com.code[1] |= (arg1&0b1)<<7
                | (arg2&0b1)<<4;
            break;
        default:
            break;

    }

    uint8_t error = 0;

    bmsCommandInit(&c, com, num, data, &error);

    // sends command to the command queue and returns the error or success code
    // TODO: check for failure 
    xQueueSend(commandQueue, &c, portMAX_DELAY); 

    // wait for callback and return its return
    // TODO: check for failure 
    xSemaphoreTake(pushSemaphore, portMAX_DELAY); 

    return error;
    
}

/* Set a multiplexer to the pin provided
 *
 *  @param mux: The number of the multiplexer to set (0 or 1)
 *  @param pin: The pin to set the multiplexer to (0 - 3)
 * 
 *  @return error code, true if command acknowlaged false if not //NONFUNCTIONAL
*/
uint8_t muxSet(uint8_t mux, uint8_t pin){
    int error = 0;
    uint8_t ack_mask = 0x08;
    uint8_t addr = 0x90 | (mux<<1);
    uint8_t comm = 0x08 | pin;
    uint8_t data[6] = { 0x60, 0x08, 0x00, 0x09, 0x70, 0x09 };
    uint8_t data_buff[6] = { 0x03, 0x00, 0x00, 0x00, 0x00, 0x00};
    data[0] |= (addr>>4)&0x0f;
    data[1] |= (addr<<4)&0xf0;
    data[3] |= (comm<<4)&0xf0;

    //taskENTER_CRITICAL(); //force thread safety if multiple i2c commands are sent in close proximity
    error = pushCommand(WRCOMM, SLAVE_COUNT, data);
    error = pushCommand(STCOMM, SLAVE_COUNT, data_buff);
    error = pushCommand(RDCOMM, SLAVE_COUNT, data_buff);
    //taskEXIT_CRITICAL();


    //Check ack/nack from buff
    //return !(data_buff[1] & ack_mask) && !(data_buff[3] & ack_mask) && !(data_buff[5] & ack_mask));
    return 1;
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
