#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MKE18F16.h"
#include "fsl_debug_console.h"
#include "System.h"

#include "gpio.h"
#include "spi.h"

#include "ltcutility.h"
#include "drivers.h"

using namespace BSP;

gpio::GPIO_port ltccsport = gpio::PortE;
uint8_t ltccspin = 6;
static constexpr uint8_t slavecount = 1;
static uint8_t txdatabuf[4 + 8*slavecount] = {0};
static uint8_t rxdatabuf[4 + 8*slavecount] = {0};
uint8_t slaves = slavecount;

void tick(void){
}

// System ticks: handler called at .1ms
// Smallest quanta is 100us
#define CLOCKHZ 60000000U
#define SYSTICK 6000U
#define us(x) 100/x
#define ms(x) 10*x

int main(void) {
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitBootPeripherals();
    SysTick_Config(SYSTICK);

    (void)rxdatabuf;
    (void)txdatabuf;

    bms::bmsinit();
    initspi();

    for(uint32_t i = 0; i < 100000; i++);

    bms::bmstransmit();

    while(1){
    }

    uint8_t data[6] = {0xfc, 0x00, 0x00, 0x00, 0x02, 0x00};
    uint8_t pec15[2] = {0};
    bms::pec15_calc(6, data, pec15);

    for(uint32_t i = 0; i < 1000U; i++) __NOP();

    // uint8_t txdata[12] = {0x00, 0x01, 0x3d, 0x6e, 0xfc, 0x00, 0x00, 0x00, 0x02, 0x00, 0xd4, 0x28};
    uint8_t txdata[12] = {0x00, 0x01, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00};
    bms::pec15_calc(2, txdata, txdata+2);
    bms::pec15_calc(6, txdata+4, txdata+10);
    uint8_t rxdata[12] = {0};

    spi::SPI& spi = spi::SPI::StaticClass();

    spi.mastertx(0, txdata, rxdata, 12);

    while(spi.xcvrs[0].transmitting);
    uint8_t txdata2[12] = {0x00, 0x02, 0x2b, 0x0a, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    spi.mastertx(0, txdata2, rxdata, 12);

    while(1) {

    }

    return 0;
}

extern "C" {
void SysTick_Handler(void){
    bms::bmstick();
}
}
