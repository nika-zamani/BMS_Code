#include "defines.h"
#include "drivers.h"
#include "cache.h"

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
    c0.callback = cancb;
    can.init(0, &c0);

    return 0;

}

void cantransmit(){

    can::CANlight& can = can::CANlight::StaticClass();

    can::CANlight::frame fVoltage;

    fVoltage.ext = 1;
    fVoltage.id = 0x314;
    fVoltage.data[0] = cache.voltageTotal & 0xff;
    fVoltage.data[1] = (cache.voltageTotal>>8) & 0xff;
    fVoltage.data[2] = (cache.voltageTotal>>16) & 0xff;

    can.tx(0, fVoltage);

}

void bmsOkOut(){
    gpio::GPIO::set(bmsokport, bmsokpin);
}

void bmsNotOkOut(){
    gpio::GPIO::clear(bmsokport, bmsokpin);
}
