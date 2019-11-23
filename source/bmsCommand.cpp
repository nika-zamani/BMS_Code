include <stdio.h>
#include <string.h>
#include <future>
#include "bmsCommand.h"

/*  Initializes data into a bms command
 *      @param c: The command to initialize
 *      @param com: The integer command number
 *      @param length: The length of each chips data
 *      @param num: The number of chips
 *      @param data: The data for the command
 */
void bmsCommandInit(bmscommand_t *c, uint8_t com[2], int length, int num, uint8_t *data, uint8_t* result, SemaphoreHandle_t semaphore)
{
    // TODO: error checking of command number
    c->command = com;
    c->size = length;
    c->num = num;
    c->data = data;
    c->result = result;
    c->semaphore = semaphore;
}

/* maps an integer command id to its corresponding command and loads said command's SPI command into the spi array.
 *      @param command: A ponter to the command to turn into a buffer
 *      @param tx: A pointer to the spi buffer array [must be at least bmsCommandSize(command) bytes large]
 *  
 *      @return An integer error code or 1 if successfull
 */  
int buildCommandBuffer(bmscommand_t *command, uint8_t *tx)
{
    int len = 0;

    memcpy(tx, command->command, 2);
    len += 2;
    pec15_calc(len, tx, &tx[len]);
    len += 2;
    
    // If the command does not require data fill the command buffer with F's
    if(command->data == NULL) {

        memset(&tx[len], 255, command->num * (command->size + 2));

        return 1;
    }

    for(int i = command->num-1; i >=0; i--)
    {
        memcpy(&tx[len], &command->data[i*command->size], command->size);
        len += command->size;
        pec15_calc(len, tx, &tx[len]);
        len += 2;
    }

    return 1;
}

/*  Gets the size of a command buffer [byte array] for this command
 *      @param c: Pointer to the command to get the size of
 * 
 *      @return The size of the command as a byte array
 */
int bmsCommandSize(bmscommand_t *c) {
    /* command code(2) + pec(2) + number of chips(num) * [command length(size) bytes + pec(2)] */
    return 4 + (c->size + 2) * c->num;
}

/*
  Calculates  and returns the CRC15
  */
void pec15_calc(uint8_t len,   //Number of bytes that will be used to calculate a PEC
        uint8_t *data,              //Array of data that will be used to calculate  a PEC
        uint8_t *pec)               // Location of PEC
{
    uint16_t remainder,addr;

    remainder = 16;//initialize the PEC
    for (uint8_t i = 0; i<len; i++) // loops for each byte in data array
    {
        addr = ((remainder>>7)^data[i])&0xff;//calculate PEC table address
        remainder = (remainder<<8)^crc15Table[addr];
        //remainder = (remainder<<8)^pgm_read_word_near(crc15Table+addr);
    }

    //The CRC15 has a 0 in the LSB so the remainder must be multiplied by 2
    pec[0] = (remainder >> 7) & 0xff; // Upper byte
    pec[1] = (remainder << 1) & 0xff; // Lower byte
}
