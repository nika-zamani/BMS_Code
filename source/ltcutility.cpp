#include <stdint.h>
#include "ltcutility.h"
#include "gpio.h"
#include "spi.h"
#include "defines.h"
#include "cache.h"

using namespace BSP;
using namespace bms;

static bmsstate_t bmsstate;
static bmscommands_t lastcommand;
static uint8_t commandOnly;
static uint8_t lastCommandOnly;
static uint8_t out;
static uint8_t lastOut;
static uint8_t extraClocks;
static bmscommands_t nextcommand;
static uint8_t txdatabuf[4 + 8*slaves] = {0};
static uint8_t rxdatabuf[4 + 8*slaves] = {0};
static uint8_t bmstimer;
static uint8_t bmson;
static uint8_t bmsawake;
static uint8_t bmscommandwaiting;
static uint8_t pecError;
static uint32_t pecErrorCount;
extern cache_t cache;

static uint32_t badcount;
static uint32_t goodcount;

static void getcommand(bmscommands_t command, uint8_t* buf);

extern uint32_t minticks;

// SPI callback for BMS. 
// Handles checking incoming PEC and moving incoming data into appropriate
// places. If lastOut is asserted, indicating most recent outgoing message was
// a write command or dataless command, callback does nothing.
void bmsspicb(){

    if(!lastOut){

        uint8_t pecrx[2];
        uint8_t i;

        // Check all incoming PEC
        for(i = 0; i < slaves; i++){
            pec15_calc(6, rxdatabuf+(8*i)+4, pecrx);
            if(pecrx[0] != rxdatabuf[8*(i+1)+2] || pecrx[1] != rxdatabuf[8*(i+1)+3]){
                pecError = 1;
                pecErrorCount++;
                return;
            }
        }

        pecError = 0;

        switch(lastcommand){

            case RDCVA: // Read cells 0-2
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
                for(i = 0; i < slaves; i++){
                    cache.gpio[(i*5)+0] = 
                        (rxdatabuf[4+(8*i)+0]) | (rxdatabuf[4+(8*i)+1]<<8);
                    cache.gpio[(i*5)+1] = 
                        (rxdatabuf[4+(8*i)+2]) | (rxdatabuf[4+(8*i)+3]<<8);
                    cache.gpio[(i*5)+2] = 
                        (rxdatabuf[4+(8*i)+4]) | (rxdatabuf[4+(8*i)+5]<<8);
                }
                break;

            case RDCOMM:
                if(rxdatabuf[4] == 0x7F) {
                    badcount++;
                    minticks++;
                } else goodcount++; 
                __NOP();

                break;

            default:
                break;

        }
    }
}


static void bmsspitransmit(){
    spi::SPI& spi = spi::SPI::StaticClass();
    if(!spi.xcvrs[0].transmitting){
        lastcommand = nextcommand;
        lastCommandOnly = commandOnly;
        lastOut = out;
        uint16_t messageSize = 4;
        if(!commandOnly) messageSize += 8*slaves;
        messageSize += extraClocks;
        extraClocks = 0;
        spi.mastertx(0, txdatabuf, rxdatabuf, messageSize);
        bmscommandwaiting = 0;
    }
}

static void bmsdriver(void){

    switch(bmsstate){

        case idle:
            if(bmson){
                // Go to wakedown
                gpio::GPIO::clear(ltccsport, ltccspin);
                bmstimer = 4; // 400us
                bmsstate = wakedown;
            }
            break;

        case wakedown:
            if(!bmstimer){
                // Go to wakeup
                gpio::GPIO::set(ltccsport, ltccspin);
                bmstimer = 1;
                bmsawake++;
                bmsstate = wakeup;
            }
            break;

        case wakeup:
            if(!bmstimer){
                if(bmsawake == slaves){
                    // Go to ready
                    bmstimer = 50; // 5ms
                    bmsstate = ready;
                } else {
                    // Go to wakedown
                    gpio::GPIO::clear(ltccsport, ltccspin);
                    bmstimer = 4;
                    bmsstate = wakedown;
                }
            }
            break;

        case ready:
            if(!bmstimer){
                // Go to idle
                bmson = 0;
                bmsawake = 0;
                bmsstate = idle;
            } else {
                if(bmscommandwaiting){
                    bmsspitransmit();
                    bmstimer = 50;
                }
            }
            break;

        default:
            break;

    }
}


void bms::tick(void){
    if(bmstimer) bmstimer--;
    bmsdriver();
}

void bms::transmit(){

    uint8_t data[6*slaves];
    memset(data, 0xff, 6*slaves);

    transmit(data);
}

void bms::wait(){
    spi::SPI& spi = spi::SPI::StaticClass();
//    while(bmscommandwaiting || spi.xcvrs[0].transmitting);
    uint8_t i, j;
    do {
    i = spi.xcvrs[0].transmitting;
    j = spi.xcvrs[0].txcount < spi.xcvrs[0].txsize;
    }while(bmscommandwaiting || (i&j));
}

void bms::transmit(uint8_t* data){

    // Clear command + PEC
    // Intent is to send an invalid message should interrupt fire inside this function
    uint8_t tempcommand[2] = {txdatabuf[0], txdatabuf[1]};
    memset(txdatabuf, 0, 4);

    
    // Copy data into tx data buffer & calculate PECs
    for(uint8_t i = 0; i < slaves; i++){
        memcpy(txdatabuf + (8*i + 4), data + (6*i), 6);
        pec15_calc(6, txdatabuf + (8*i + 4), txdatabuf + (8*(i+1) + 2));
    }

    memcpy(txdatabuf, tempcommand, 2);
    pec15_calc(2, txdatabuf, txdatabuf + 2);

    bmscommandwaiting = 1;
    bmson = 1;

    if(bmsstate == ready) bmsspitransmit();
}

// read all cells and sum of cells 
// defaults: MD = 0b10 (7kHz mode), DCP = 1 (discharge permitted)
void bms::adcvsc(uint8_t MD, uint8_t DCP){

    nextcommand = ADCVSC;
    commandOnly = 1;
    out = 1;

    getcommand(ADCVSC, txdatabuf);
    txdatabuf[0] |= (MD&0b10)>>1;
    txdatabuf[1] |= (MD&0b01)<<7;
    txdatabuf[1] |= (DCP&0b1)<<4;
    transmit();

}

// read all cells, sum of cells, and gpio 1&2
// defaults: MD = 0b10 (7kHz mode), DCP = 1 (discharge permitted)
void bms::adcvax(uint8_t MD, uint8_t DCP){

    nextcommand = ADCVAX;
    commandOnly = 1;
    out = 1;

    getcommand(ADCVAX, txdatabuf);
    txdatabuf[0] |= (MD&0b10)>>1;
    txdatabuf[1] |= (MD&0b01)<<7;
    txdatabuf[1] |= (DCP&0b1)<<4;
    transmit();
}


void bms::rdcva(){

    nextcommand = RDCVA;
    commandOnly = 0;
    out = 0;

    getcommand(RDCVA, txdatabuf);
    transmit();

}

void bms::rdcvb(){

    nextcommand = RDCVB;
    commandOnly = 0;
    out = 0;

    getcommand(RDCVB, txdatabuf);
    transmit();

}

void bms::rdcvc(){

    nextcommand = RDCVC;
    commandOnly = 0;
    out = 0;

    getcommand(RDCVC, txdatabuf);
    transmit();

}

void bms::rdcvd(){

    nextcommand = RDCVD;
    commandOnly = 0;
    out = 0;

    getcommand(RDCVD, txdatabuf);
    transmit();

}

void bms::rdauxa(){

    nextcommand = RDAUXA;
    commandOnly = 0;
    out = 0;

    getcommand(RDAUXA, txdatabuf);
    transmit();
}

void bms::wrcomm(uint8_t* data){

    nextcommand = WRCOMM;
    commandOnly = 0;
    out = 1;

    getcommand(WRCOMM, txdatabuf);
    transmit(data);

}

void bms::rdcomm(){

    nextcommand = RDCOMM;
    commandOnly = 0;
    out = 0;

    getcommand(RDCOMM, txdatabuf);
    transmit();

}

void bms::stcomm(uint8_t bytes){

    nextcommand = STCOMM;
    commandOnly = 1;
    out = 1;
    extraClocks += bytes*3;

    getcommand(STCOMM, txdatabuf);
    transmit();

}

uint8_t bms::init(){

    bmsstate = idle;
    bmstimer = 0;
    bmson = 0;
    bmsawake = 0;
    bmscommandwaiting = 0;
    extraClocks = 0;
    badcount = 0;
    goodcount = 0;

    return 0;

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

