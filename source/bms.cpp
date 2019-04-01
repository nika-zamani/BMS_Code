/* System */
#include "pin_mux.h"
#include "clock_config.h"
#include "MKE18F16.h"
#include "fsl_debug_console.h"

/* BSP */
#include "System.h"
#include "gpio.h"
#include "spi.h"

/* Project */
#include "defines.h"
#include "ltcutility.h"
#include "utility.h"
#include "drivers.h"
#include "cache.h"

using namespace BSP;

//gpio::GPIO_port ltccsport = gpio::PortE;
//uint8_t ltccspin = 6;

cache_t cache = {0};

volatile uint32_t ticks;

void tick(void){
}

typedef void (*state_t)(void);
static state_t state;
void stateNormal(void);
void stateError(void);

uint8_t manage(void){

    // read volts
    
    bms::adcvsc();
    bms::wait();
    bms::rdcva();
    bms::wait();
    bms::rdcvb();
    bms::wait();
    bms::rdcvc();
    bms::wait();
    bms::rdcvd();
    bms::wait();

    uint16_t rxmask;
    do{
        rxmask = 0x7ff;
        for(uint8_t i = 0; i < slaves; i++)
            rxmask &= cache.adcRxMask[i];
    }while(rxmask != 0x7ff);

    diagnoseVolts();

    if(!cache.allok) {
        return 1;
    }

    // read temps
    

    return 0;
    
}


void stateNormal(void){

    while(ticks);
    ticks = ms(500);
    if(manage()){
       state = stateError;
       return;
    }

}

void bmsNotOk(void){
    // turn off ok signal
}

void stateError(void){

    bmsNotOk();

    while(ticks);
    ticks = ms(3000);

    manage();

}


int main(void) {
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    SysTick_Config(SYSTICK);

    cache.allok = 1;
    cache.voltok = 1;
    cache.tempok = 1;

    ticks = 0;

    bms::init();
    initspi();

    state = stateNormal;
    
    ticks = ms(500);

    while(1){

        state();

    }
        
    return 0;
}

extern "C" {
void SysTick_Handler(void){
    if(ticks) ticks--;
    bms::tick();
}
}
