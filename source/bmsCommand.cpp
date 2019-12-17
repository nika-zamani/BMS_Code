#include "bmsCommand.h"

void bmsCommandInit(bmscommand_t *c, bmscom_t com, int num,
        uint8_t *data, uint8_t* result, SemaphoreHandle_t semaphore)
{
    c->c= com;
    c->num = num;
    c->data = data;
    c->result = result;
    c->semaphore = semaphore;
}

int bmsCommandSize(bmscommand_t *c) {
    /* command code(2) + pec(2) + number of chips(num) * 
           [command length(size) bytes + pec(2)] */
    return 4 + (c->size + 2) * c->num;
}

void pec15_calc(uint8_t len,    
        uint8_t *data,          
        uint8_t *pec)           
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
