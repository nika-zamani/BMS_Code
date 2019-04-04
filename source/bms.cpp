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
#include "actions.h"

using namespace BSP;

cache_t cache;
/*
void switchMux(uint8_t mux, uint8_t pin){
    uint8_t addr = 0x90 | (mux<<1);
    uint8_t comm = 0x08 | pin;
    uint8_t data[6] = { 0x60, 0x08, 0x00, 0x09, 0x70, 0x09 };
    data[0] |= (addr>>4)&0x0f;
    data[1] |= (addr<<4)&0xf0;
    data[3] |= (comm<<4)&0xf0;
    bms::wrcomm(data);
    bms::wait();
    delay(3);
    bms::stcomm(2);
    bms::wait();
}

uint8_t manage(void){

    // read volts
    
    bms::rdauxa();
    bms::wait();
    delay(1);
    bms::rdcva();
    bms::wait();
    delay(1);
    bms::rdcvb();
    bms::wait();
    delay(1);
    bms::rdcvc();
    bms::wait();
    delay(1);
    bms::rdcvd();
    bms::wait();
    delay(1);

    // start adc conversion to be read on next cycle...
    // registers cleared when watchdog times out, approx 2s
    bms::adcvax();
    bms::wait();

    uint16_t rxmask = {0};
    for(uint8_t i = 0; i < cells; i++){
        rxmask |= 1<<i;
    }
    uint8_t done = 0;
    while(1){
        done = 0;
        for(uint8_t i = 0; i < slaves; i++)
            if(cache.adcRxMask[i] == rxmask) done++;
        if(done==slaves){ 
            diagnoseVolts();
            break;
        }
        if(ticks == 0) return 0;
    }

    if(!cache.allok) {
        return 1;
    }

    // read temps
    
    delay(ms(2));
    //pin = (pin+1)%8;
    pin = (pin+1)%8;
    switchMux(1, pin);

    return 0;
    
}
*/

void cacheinit(){
    memset(&cache, 0, sizeof(cache_t));
    cache.allok = 1;
    cache.voltok = 1;
    cache.tempok = 1;
}

int main(void) {
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    SysTick_Config(SYSTICK);

    cacheinit();
    spiinit();
    actinit();

    actwait(ms(500));
    bms::adcvax();
    bms::rdcfga();

    while(1){
        actexec();
    }
/*
    action_t a;
    a.a = wait;
    a.wait = ms(500);
    actadd(a);
    a.a = csdown;
    actadd(a);
    a.a = wait;
    a.wait = ms(3);
    actadd(a);
    a.a = csup;
    actadd(a);
    a.a = spitx;
    a.data[0] = 0xaa;
    a.len = 3;
    a.comm = bms::PLADC;
    actadd(a);

    while(1){
        actexec();
    }

    // setup: 
    // - set refup to 1 to keep reference voltage on
    // - start ADC conversion and wait for completion
    uint8_t confdat[6];
    memset(confdat, 0, 6);
    confdat[0] = 0xfc;
    bms::adcvax();
    bms::wait();
    delay(ms(2));

    while(1){

        // repeat process at 2Hz
        ticks = ms(500);
        while(ticks);

        bms::wrcfga(confdat);
        bms::wait();
        bms::rdcfga();
        bms::wait();

        pin = (pin+1)%8;
        switchMux(1, pin);

        // wait for mux to settle, then trigger conversion
        delay(ms(50));
        bms::adcvax();
        // wait for conversion to finish, then read from auxa register
        delay(ms(10));

        bms::rdauxa();
        bms::wait();
        delay(1);
        bms::rdcva();
        bms::wait();
        delay(1);
        bms::rdcvb();
        bms::wait();
        delay(1);
        bms::rdcvc();
        bms::wait();
        delay(1);
        bms::rdcvd();
        bms::wait();
        delay(1);
        uint16_t rxmask = {0};
        for(uint8_t i = 0; i < cells; i++){
            rxmask |= 1<<i;
        }
        uint8_t done = 0;
        while(1){
            done = 0;
            for(uint8_t i = 0; i < slaves; i++)
                if(cache.adcRxMask[i] == rxmask) done++;
            if(done==slaves){ 
                diagnoseVolts();
                break;
            }
            if(ticks == 0) break;
        }
    }
        
    return 0;
    */
}

extern "C" {
void SysTick_Handler(void){
    acttick();
}
}
