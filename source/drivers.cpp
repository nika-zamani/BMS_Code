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
    static can::CANlight::frame chargerf;

    switch(f.id){
        case 0x18ff50e5:
            chargerf = f;
            cache.charger = 100;
            cache.timeout.charger = 0;
            cache.chargerVolts = (f.data[0]<<8)+f.data[1];
            cache.chargerAmps = (f.data[2]<<8)+f.data[3];
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
    c0.baudRate = 250000UL;
    c0.callback = cancb;
    can.init(0, &c0);
    return 0;

}

void can_uptime_tx(){

    static uint32_t uptime = 0;

    uptime++;
    
    can::CANlight::frame f;
    f.ext = 1;
    f.id = CANBASE;
    f.dlc = 4;
    f.data[0] = (uptime>>0) & 0xff;
    f.data[1] = (uptime>>8) & 0xff;
    f.data[2] = (uptime>>16) & 0xff;
    f.data[3] = (uptime>>24) & 0xff;

    can::CANlight::StaticClass().tx(0, f);

}

void cantransmit(){

    can::CANlight& can = can::CANlight::StaticClass();

    can::CANlight::frame fData;

    fData.ext = 1;
    fData.id = CANBASE+CANDATAOFFSET;
    
    // Load voltage data
    uint32_t v = cache.voltageTotal / 10000;
    fData.data[0] = v & 0xff;
    fData.data[1] = (v>>8) & 0xff;

    // Load temperature data
    uint16_t i = 0;
    while(i < TempQa-1 && (cache.tempMax/10) > templut[i]){
        i++;
    }
    fData.data[2] = (i+TempMin) & 0xff;
    fData.data[3] = ((i+TempMin)>>8) & 0xff;

    // Load state data

    fData.data[4] = (1^cache.allok) << 0 |
        (1^cache.voltok) << 1 |
        (1^cache.tempok) << 2 |
        (1^cache.commsok) << 3;

    can.tx(0, fData);

    can::CANlight::frame ftemps;
    ftemps.ext = 1;

    uint8_t j, k;

    for(i = 0; i <= (thermistors*slaves)/8; i++){

        ftemps.id = CANBASE+CANTEMPSOFFSET+i;

        j = 0;

        while(j < 8 && j + (i*8) < thermistors*slaves){
            k = 0;
            while(k < TempQa-1 && (cache.temps[(i*8)+j]/10) > templut[k]){
                k++;
            }
            ftemps.data[j] = k+TempMin;
            j++;
            ftemps.dlc = j;
        }

        can.tx(0, ftemps);

    }

    can::CANlight::frame fvolts;
    fvolts.ext = 1;

    for(i = 0; i <= (thermistors*cells)/8; i++){

        fvolts.id = CANBASE+CANVOLTSOFFSET+i;

        j = 0;

        while(j < 8 && j + (i*8) < cells*slaves){
            fvolts.data[j] = (((cache.volts[j+(i*8)]-voltageLimitLower)*100)/voltageLimitUpper);
            j++;
            fvolts.dlc = j;
        }

        can.tx(0, fvolts);

    }


}

void bmsOkOut(){
    gpio::GPIO::set(bmsokport, bmsokpin);
}

void bmsNotOkOut(){
    gpio::GPIO::clear(bmsokport, bmsokpin);
}
