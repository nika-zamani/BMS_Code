#include "drivers.h"

using namespace BSP;

extern gpio::GPIO_port ltccsport;
extern uint8_t ltccspin;

uint8_t initspi(){
    spi::spi_config conf;
    spi::SPI::ConstructStatic(&conf);
    spi::SPI& spi = spi::SPI::StaticClass();

    spi::SPI::masterConfig mconf;
    mconf.csport = ltccsport;
    mconf.cspin = ltccspin;
    spi.initMaster(0, &mconf);

    return 0;
}


