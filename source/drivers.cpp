#include "defines.h"
#include "drivers.h"
#include "cache.h"
#include "temps.h"

extern cache_t cache;

using namespace BSP;

void bmsspicb(void);

uint8_t spiinit(){
    spi::spi_config conf;
    conf.callbacks[0] = bmsspicb;
    spi::SPI::ConstructStatic(&conf);
    spi::SPI& spi = spi::SPI::StaticClass();

    spi::SPI::masterConfig mconf;
    mconf.baudRate = 500000U;
    mconf.csport = ltccsport;
    mconf.cspin = ltccspin;
    spi.initMaster(0, &mconf);

    return 0;
}

void cancb(){

    can::CANlight::frame f = can::CANlight::StaticClass().readrx(0);

    switch(f.id){
        case 0x18ff50e5:
            cache.charger = 1;
            cache.timeout.charger = 0;
            break;

        default:
            break;

    }

}

uint8_t caninit(){

    can::canlight_config c;
    can::CANlight::ConstructStatic(&c);
    can::CANlight& can = can::CANlight::StaticClass();

    can::CANlight::canx_config c0;
    c0.baudRate = 1000000UL;
    c0.callback = cancb;
    can.init(0, &c0);

    return 0;

}

#define CANBASE 0x314

static void cantxCommsE(){
    
    can::CANlight::frame f;
    f.ext = 1;
    f.id = CANBASE+3;
    f.data[0] = (uint8_t)cache.commsEC;
    f.data[1] = cache.commsED;

    can::CANlight::StaticClass().tx(0, f);

}

static void cantxTempE(){
    
    can::CANlight::frame f;
    f.ext = 1;
    f.id = CANBASE+2;
    f.data[0] = (uint8_t)cache.tempEC;
    f.data[1] = cache.tempED;

    can::CANlight::StaticClass().tx(0, f);

}

static void cantxVoltE(){
    
    can::CANlight::frame f;
    f.ext = 1;
    f.id = CANBASE+1;
    f.data[0] = (uint8_t)cache.voltEC;
    f.data[1] = cache.voltED[0];
    f.data[2] = cache.voltED[1];
    f.data[3] = cache.voltED[2];

    can::CANlight::StaticClass().tx(0, f);

}

void cantransmit(){

    can::CANlight& can = can::CANlight::StaticClass();

    can::CANlight::frame fData;

    fData.ext = 1;
    fData.id = 0x314;
    
    // Load voltage data
    uint32_t v = cache.voltageTotal / 10000;
    fData.data[0] = v & 0xff;
    fData.data[1] = (v>>8) & 0xff;

    // Load temperature data
    uint16_t i = 0;
    while(cache.tempMax > templut[i] && i < TempQa){
        i++;
    }
    fData.data[2] = (i+TempMin) & 0xff;
    fData.data[3] = ((i+TempMin)>>8) & 0xff;

    can.tx(0, fData);

    if((uint8_t)cache.commsEC) cantxCommsE();
    if((uint8_t)cache.voltEC) cantxVoltE();
    if((uint8_t)cache.tempEC) cantxTempE();

}

void bmsOkOut(){
    gpio::GPIO::set(bmsokport, bmsokpin);
}

void bmsNotOkOut(){
    gpio::GPIO::clear(bmsokport, bmsokpin);
}
