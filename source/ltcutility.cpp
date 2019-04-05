#include <stdint.h>
#include "ltcutility.h"
#include "gpio.h"
#include "spi.h"
#include "defines.h"
#include "cache.h"
#include "actions.h"

using namespace BSP;
using namespace bms;

extern cache_t cache;

/* bms status and control */
static bmscommands_t lastcommand;
static uint8_t txdatabuf[commlen] = {0};
static uint8_t rxdatabuf[commlen] = {0};
static uint8_t tmpbuf[commlen+1] = {0}; // for temporary use, extra byte for i2c
static uint8_t pecError;
static uint32_t pecErrorCount;
static uint8_t bmstransmitting;

// get the corresponding command bytes (thanks andrew!)
static void getcommand(bmscommands_t command, uint8_t* buf);

// move command and outgoing write data into buffer and calculate PECs
static void dataload(bmscommands_t c, uint8_t* src, uint8_t* dest){
    getcommand(c, dest);
    pec15_calc(2, dest, dest+2);
    for(uint8_t i = 0; i < slaves; i++){
        memcpy(dest+4+(8*i), src+(6*i), 6);
        pec15_calc(6, dest+4+(8*i), dest+2+(8*(i+1)));
    }
}

// fill space with 0xff for read commands
static void datafill(bmscommands_t c, uint8_t* dest, uint16_t len){
    getcommand(c, dest);
    pec15_calc(2, dest, dest+2);
    memset(dest+4, 0xff, len);
}

// common setup function for writing commands
static void spiwrite(bmscommands_t c, uint8_t* data){
    if(actqcheck(2)) return;
    dataload(c, data, tmpbuf);
    actspitx(c, tmpbuf, commlen);
    actspiwait();
}

// common setup function for reading commands
static void spiread(bmscommands_t c){
    if(actqcheck(2)) return;
    datafill(c, tmpbuf, commlen);
    actspitx(c, tmpbuf, commlen);
    actspiwait();
}

// cell group register read
void bms::rdcva(void){ spiread(RDCVA); }
void bms::rdcvb(void){ spiread(RDCVB); }
void bms::rdcvc(void){ spiread(RDCVC); }
void bms::rdcvd(void){ spiread(RDCVD); }

// aux register read
void bms::rdauxa(void){ spiread(RDAUXA); }
void bms::rdstata(void){ spiread(RDSTATA); }

// write to conf registers
void bms::wrcfga(uint8_t* data){
    spiwrite(WRCFGA, data);
    // need to wait more?
}

// read conf registers
void bms::rdcfga(void){
    spiread(RDCFGA);
}

// adc cells + total + gpio1,2
void bms::adcvax(uint8_t MD, uint8_t DCP){
    if(actqcheck(3)) return;
    getcommand(ADCVAX, tmpbuf);
    tmpbuf[0] |= (MD&0b10)>>1;
    tmpbuf[1] |= (MD&0b01)<<7;
    tmpbuf[1] |= (DCP&0b1)<<4;
    pec15_calc(2, tmpbuf, tmpbuf+2);
    actspitx(ADCVAX, tmpbuf, 4);
    actspiwait();
    actwait(ms(3));
}

// adc cells + total
void bms::adcvsc(uint8_t MD, uint8_t DCP){
    if(actqcheck(3)) return;
    getcommand(ADCVSC, tmpbuf);
    tmpbuf[0] |= (MD&0b10)>>1;
    tmpbuf[1] |= (MD&0b01)<<7;
    tmpbuf[1] |= (DCP&0b1)<<4;
    pec15_calc(2, tmpbuf, tmpbuf+2);
    actspitx(ADCVSC, tmpbuf, 4);
    actspiwait();
    actwait(ms(3));
}

void wrcomm(uint8_t* data){
    spiwrite(WRCOMM, data);
    actwait(1);
}

void rdcomm(){
    spiread(RDCOMM);
}

void stcomm(uint8_t bytes){
    if(actqcheck(2)) return;
    getcommand(STCOMM, tmpbuf);
    pec15_calc(2, tmpbuf, tmpbuf+2);
    // for each i2c byte, add 3 bytes of 0xff for 24 clocks
    memset(tmpbuf+4, 0xff, bytes*3);
    actspitx(STCOMM, tmpbuf, 4+(bytes*3));
    actspiwait();
}

static uint8_t peccheck(){
    uint8_t pecrx[2];
    for(uint8_t i = 0; i < slaves; i++){
        pec15_calc(6, rxdatabuf+(8*i)+4, pecrx);
        if(pecrx[0] != rxdatabuf[8*(i+1)+2] || pecrx[1] != rxdatabuf[8*(i+1)+3]){
            pecError = 1;
            pecErrorCount++;
            return 1;
        }
    }
    return 0;
}


// SPI callback for BMS. 
// Handles checking incoming PEC and moving incoming data into appropriate
// places. If lastOut is asserted, indicating most recent outgoing message was
// a write command or dataless command, callback does nothing.
void bmsspicb(){

    bmstransmitting = 0;

    uint8_t i,j;

    switch(lastcommand){

        case RDCVA: // Read cells 0-2
            if(peccheck()) return;
            for(i = 0; i < slaves; i++){
                cache.volts[(i*cells)+0] = (rxdatabuf[4+(8*i)+0]) | 
                    (rxdatabuf[4+(8*i)+1]<<8);
                cache.adcRxMask[i] |= 1<<0;
                cache.volts[(i*cells)+1] = (rxdatabuf[4+(8*i)+2]) | 
                    (rxdatabuf[4+(8*i)+3]<<8);
                cache.adcRxMask[i] |= 1<<1;
                cache.volts[(i*cells)+2] = (rxdatabuf[4+(8*i)+4]) | 
                    (rxdatabuf[4+(8*i)+5]<<8);
                cache.adcRxMask[i] |= 1<<2;
            }
            break;

        case RDCVB: // Read cells 3-5
            if(peccheck()) return;
            for(i = 0; i < slaves; i++){
                cache.volts[(i*cells)+3] = (rxdatabuf[4+(8*i)+0]) | 
                    (rxdatabuf[4+(8*i)+1]<<8);
                cache.adcRxMask[i] |= 1<<3;
                cache.volts[(i*cells)+4] = (rxdatabuf[4+(8*i)+2]) | 
                    (rxdatabuf[4+(8*i)+3]<<8);
                cache.adcRxMask[i] |= 1<<4;
                cache.volts[(i*cells)+5] = (rxdatabuf[4+(8*i)+4]) | 
                    (rxdatabuf[4+(8*i)+5]<<8);
                cache.adcRxMask[i] |= 1<<5;
            }
            break;

        case RDCVC: // Read cells 6-8
            if(peccheck()) return;
            for(i = 0; i < slaves; i++){
                cache.volts[(i*cells)+6] = (rxdatabuf[4+(8*i)+0]) | 
                    (rxdatabuf[4+(8*i)+1]<<8);
                cache.adcRxMask[i] |= 1<<6;
                cache.volts[(i*cells)+7] = (rxdatabuf[4+(8*i)+2]) | 
                    (rxdatabuf[4+(8*i)+3]<<8);
                cache.adcRxMask[i] |= 1<<7;
                cache.volts[(i*cells)+8] = (rxdatabuf[4+(8*i)+4]) | 
                    (rxdatabuf[4+(8*i)+5]<<8);
                cache.adcRxMask[i] |= 1<<8;
            }
            break;

        case RDCVD: // Read cells 9-11
            if(peccheck()) return;
            for(i = 0; i < slaves; i++){
                if(cells >= 10) {
                    cache.volts[(i*cells)+9] = 
                        (rxdatabuf[4+(8*i)+0]) | (rxdatabuf[4+(8*i)+1]<<8);
                    cache.adcRxMask[i] |= 1<<9;
                }
                if(cells >= 11) {
                    cache.volts[(i*cells)+10] = 
                        (rxdatabuf[4+(8*i)+2]) | (rxdatabuf[4+(8*i)+3]<<8);
                    cache.adcRxMask[i] |= 1<<10;
                }
                if(cells >= 12) {
                    cache.volts[(i*cells)+11] = 
                        (rxdatabuf[4+(8*i)+4]) | (rxdatabuf[4+(8*i)+5]<<8);
                    cache.adcRxMask[i] |= 1<<11;
                }
            }
            break;

        case RDAUXA: //read gpio 1-3
            if(peccheck()) return;
            for(i = 0; i < slaves; i++){
                cache.gpio[(i*5)+0] = 
                    (rxdatabuf[4+(8*i)+0]) | (rxdatabuf[4+(8*i)+1]<<8);
                cache.gpio[(i*5)+1] = 
                    (rxdatabuf[4+(8*i)+2]) | (rxdatabuf[4+(8*i)+3]<<8);
                cache.gpio[(i*5)+2] = 
                    (rxdatabuf[4+(8*i)+4]) | (rxdatabuf[4+(8*i)+5]<<8);
            }
            break;

        case RDSTATA:
            if(peccheck()) return;
            cache.voltageTotal = 0;
            for(i = 0; i < slaves; i++){
                for(j = 0; j < 6; j++)
                    cache.statA[i][j] = rxdatabuf[4+(8*i)+j];
                cache.voltageTotal += (cache.statA[i][1]<<8)+cache.statA[i][0];
            }
            cache.voltageTotal *= 20;
            break;

        case RDCOMM:
            if(peccheck()) return;
            for(i = 0; i < slaves; i++)
                for(j = 0; j < 6; j++)
                    cache.comm[i][j] = rxdatabuf[4+(8*i)+j];
            break;

        case RDCFGA:
            if(peccheck()) return;
            for(i = 0; i < slaves; i++)
                for(j = 0; j < 6; j++)
                    cache.configA[i][j] = rxdatabuf[4+(8*i)+j];
            break;

        default:
            break;

    }

}


static void bmsspitransmit(uint16_t len){
    spi::SPI& spi = spi::SPI::StaticClass();
    if(!spi.xcvrs[0].transmitting){
        spi.mastertx(0, txdatabuf, rxdatabuf, len);
    }
}

uint8_t bms::spistatus(){
    if(bmstransmitting) return 1;
//    spi::SPI& spi = spi::SPI::StaticClass();
//    if((spi.xcvrs[0].transmitting && spi.xcvrs[0].txcount < spi.xcvrs[0].txsize) ||
//        bmstransmitting) return 1;
    return 0;
}

void bms::transmit(uint8_t* data, uint8_t len, bmscommands_t comm){
    bmstransmitting = 1;
    lastcommand = comm;
    memcpy(txdatabuf, data, len);
    bmsspitransmit(len);
}

/*
  Calculates  and returns the CRC15
  */
void bms::pec15_calc(uint8_t len,   //Number of bytes that will be used to calculate a PEC
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

// Converts BMS command name to bytes, stored into buf
static void getcommand(bmscommands_t command, uint8_t* buf){

    switch(command) {

        case WRCFGA:
            buf[0] = 0x00;
            buf[1] = 0x01;
            break;
	
        case WRCFGB:
            buf[0] = 0x00;
            buf[1] = 0x24;
            break;

        case RDCFGA:
            buf[0] = 0x00;
            buf[1] = 0x02;
            break;

        case RDCFGB:
            buf[0] = 0x00;
            buf[1] = 0x26;
            break;

        case RDCVA:		//Read Voltage Group A
            buf[0] = 0x00;
            buf[1] = 0x04;
            break;
			
		case RDCVB:		//Read Voltage Group B
            buf[0] = 0x00;
            buf[1] = 0x06;
            break;
			
		case RDCVC:		//Read Voltage Group C
            buf[0] = 0x00;
            buf[1] = 0x08;
            break;
			
		case RDCVD:		//Read Voltage Group D
            buf[0] = 0x00;
            buf[1] = 0x0a;
            break;

        case RDCVE:
            buf[0] = 0x00;
            buf[1] = 0x09;
            break;

        case RDCVF:
            buf[0] = 0x00;
            buf[1] = 0x0b;
			
		case RDAUXA:	//Read Auxillary Register Group A
            buf[0] = 0x00;
            buf[1] = 0x0c;
            break;
			
		case RDAUXB:	//Read Auxillary Register Group B
            buf[0] = 0x00;
            buf[1] = 0x0e;
            break;

        case RDAUXC:
            buf[0] = 0x00;
            buf[1] = 0x0d;
            break;

        case RDAUXD:
            buf[0] = 0x00;
            buf[1] = 0x0f;
            break;
			
		case RDSTATA:	//Read Status Register Group B
            buf[0] = 0x00;
            buf[1] = 0x10;
            break;
			
		case RDSTATB:	//Read Status Register Group B
            buf[0] = 0x00;
            buf[1] = 0x12;
            break;
			
		case WRSCTRL:	//Write S Control Register Group
            buf[0] = 0x00;
            buf[1] = 0x14;
            break;
			
		case WRPWM:	//Write PWM Register Group
            buf[0] = 0x00;
            buf[1] = 0x20;
            break;

        case WRPSB:
            buf[0] = 0x00;
            buf[1] = 0x1c;
            break;
			
		case RDSCTRL:	//Read S Control Register Group
            buf[0] = 0x00;
            buf[1] = 0x16;
            break;
			
		case RDPWM:	//Read PWM Register Group
            buf[0] = 0x00;
            buf[1] = 0x22;
            break;

        case RDPSB:
            buf[0] = 0x00;
            buf[1] = 0x1e;
            break;
			
		case STSCTRL:	//Start S Control Pulsing and Poll Status
            buf[0] = 0x00;
            buf[1] = 0x19;
            break;
			
		case CLRSCTRL:	//Clear S Control Register Group
            buf[0] = 0x00;
            buf[1] = 0x18;
            break;
			
		case ADCV:	//Start Cell Voltage ADC Conversion and Poll Status (DCP = 0)
            buf[0] = 0x03;
            buf[1] = 0x60;
            break;
			
		case ADOW:	//Start Open Wire ADC Conversion and Poll Status (PUP = 0)(DCP = 0)
            buf[0] = 0x03;
            buf[1] = 0x28;
            break;
			
		case CVST:	//Start Self Test Cell Voltage Conversion and Poll Status (ST[0] = ST[1] = 0)
            buf[0] = 0x03;
            buf[1] = 0x07;
            break;
			
		case ADOL:	//Start Overlap Measurement of Cell 7 Voltage (DCP = 0)
            buf[0] = 0x03;
            buf[1] = 0x00;
            break;
			
		case ADAX:	//Start GPIOs ADC Conversion and Poll Status (CHG[0:2] = 0)
            buf[0] = 0x05;
            buf[1] = 0x60;
            break;
			
		case ADAXD:	//Start GPIOs ADC Conversion With Digital Redundancy and Poll Status (CHG[0:2] = 0)
            buf[0] = 0x05;
            buf[1] = 0x00;
            break;
		
		case AXST:	//Start Self Test GPIOs Conversion and Poll Status (ST[0] = ST[1] = 0)
            buf[0] = 0x05;
            buf[1] = 0x07;
            break;
		
		case ADSTAT:	//Start Status Group ADC Conversion and Poll Status (CHST[0:2] = 0)
            buf[0] = 0x05;
            buf[1] = 0x68;
            break;
		
		case ADSTATD:	//Start Status Group ADC Conversion With Digital Redundancy and Poll Status (CHST[0:2] = 0)
            buf[0] = 0x05;
            buf[1] = 0x08;
            break;
		
		case STATST:	//Start Self Test Status Group Conversion and Poll Status (ST[0] = ST[1] = 0)
            buf[0] = 0x05;
            buf[1] = 0x0f;
            break;
			
		case ADCVAX:	//Start Combined Cell Voltage and GPIO1, GPIO2 Conversion and Poll Status (DCP = 0)
            buf[0] = 0x05;
            buf[1] = 0x6f;
            break;

        case ADCVSC:
            buf[0] = 0x04;
            buf[1] = 0x67;
            break;
			
		case CLRCELL:	//Clear Cell Voltage Register Groups
            buf[0] = 0x07;
            buf[1] = 0x11;
            break;

        case CLRAUX:
            buf[0] = 0x07;
            buf[1] = 0x12;
            break;
			
		case  CLRSTAT:	//Clear Status Register Groups
            buf[0] = 0x07;
            buf[1] = 0x13;
            break;
			
		case PLADC:	//Poll ADC Conversion Status
            buf[0] = 0x07;
            buf[1] = 0x14;
            break;
			
		case DIAGN:	//Diagnose MUX and Poll Status
            buf[0] = 0x07;
            buf[1] = 0x15;
            break;
			
		case WRCOMM:	//Write COMM Register Group
            buf[0] = 0x07;
            buf[1] = 0x21;
            break;
			
		case RDCOMM:	//Read COMM Register Group
            buf[0] = 0x07;
            buf[1] = 0x22;
            break;
			
		case STCOMM:	//Start I2C /SPI Communication 
            buf[0] = 0x07;
            buf[1] = 0x23;
            break;

    }

}

