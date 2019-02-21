#define slaves 2
// System ticks: handler called at .1ms
// Smallest quantum is 100us
#define CLOCKHZ 60000000U
#define SYSTICK 6000U
#define us(x) 100/x
#define ms(x) 10*x

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
#include "cache.h"

using namespace BSP;

gpio::GPIO_port ltccsport = gpio::PortE;
uint8_t ltccspin = 6;

static cache_t cache = {0};

volatile uint32_t ticks;

void tick(void){
}

void manage(void){

    // read volts
    
    bms::adcvsc();
    
    bms::wait();
   
    bms::rdcva();

    // read temps
    
}



int main(void) {
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitBootPeripherals();
    SysTick_Config(SYSTICK);

    cache.allok = 1;


    ticks = 0;

    bms::init();
    initspi();

    for(uint32_t i = 0; i < 100000; i++);

    manage();

    while(1);
    uint8_t data[12] = {0xfc, 0x00, 0x00, 0x00, 0x01, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x01, 0x00};
    /*
    bms::transmit(bms::ADCVSC);
    ticks = 2000;
    while(ticks);
    bms::transmit(bms::RDCVA);

    while(1);
    bms::transmit(bms::writeConfig, data);
    bms::wait();
    bms::transmit(bms::readConfig);
    */
    uint16_t i;
    while(1){
        i++;
        i = i % 8;
        data[4] = 1<<i;
        data[10] = 1<<i;
        //bms::transmit(bms::WRCFGA, data);
        for(uint32_t j = 0; j < 100000UL; j++);
        
    }

    return 0;
}

extern "C" {
void SysTick_Handler(void){
    if(ticks) ticks--;
    bms::tick();
}
}
