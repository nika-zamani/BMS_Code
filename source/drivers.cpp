#include "defines.h"
#include "drivers.h"

using namespace BSP;

// CS pin defined in bms.cpp
//extern gpio::GPIO_port ltccsport;

//extern uint8_t ltccspin;

void bmsspicb(void);

uint8_t initspi(){
    spi::spi_config conf;
    conf.callbacks[0] = bmsspicb;
    spi::SPI::ConstructStatic(&conf);
    spi::SPI& spi = spi::SPI::StaticClass();

    spi::SPI::masterConfig mconf;
    mconf.baudRate = 1000000U;
    mconf.csport = ltccsport;
    mconf.cspin = ltccspin;
    spi.initMaster(0, &mconf);

    return 0;
}


