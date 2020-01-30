#include "bmsCommand.h"

void bmsCommandInit(bmscommand_t *c, bmscom_t com, int num, uint8_t *data, uint8_t *error)
{
    c->c= com;
    c->num = num;
    c->data = data;
    c->error = error;

    if(com.combb == combbTx){
        c->len = 4 + (8 * num);
    } else if(com.combb == combbRx){
        c->len = 4 + (8 * num);
    } else if(com.combb == combbDir){
        c->len = 4;
    } else if(com.combb == combbClk){
        c->len = 4 + ((3 * (*data))); // TODO: check this
    } else {
        c->len = 0;
    }
}

int combbTx(bmscommand_t* c, uint8_t* buf){
    int len = 0;
    memcpy(buf, c->c.code, 2);
    pec15_calc(2, buf, buf+2);
    len += 4;
    for(int i = 0; i < c->num; i++){
        memcpy(buf + len, c->data + (6*i), 6);
        pec15_calc(6, buf + len, buf + len + 6);
        len += 8;
    }
    return 0;
}

int combbRx(bmscommand_t* c, uint8_t* buf){
    memcpy(buf, c->c.code, 2);
    pec15_calc(2, buf, buf+2);
    memset(buf+4, 0xff, 8*c->num);
    return 0;
}

int combbDir(bmscommand_t* c, uint8_t* buf){
    memcpy(buf, c->c.code, 2);
    pec15_calc(2, buf, buf+2);
    return 0;
}

int combbClk(bmscommand_t* c, uint8_t* buf){
    memcpy(buf, c->c.code, 2);
    pec15_calc(2, buf, buf+2);
    memcpy(buf+4, 0xff, c->len - 4); //TODO: check this should fill entire space with f's
    return 0;
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
