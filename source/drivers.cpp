#include "defines.h"
#include "drivers.h"

using namespace BSP;

void bmsspicb(void);

uint8_t spiinit(){
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


