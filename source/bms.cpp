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
#include "bmsutil.h"
#include "utility.h"
#include "drivers.h"
#include "cache.h"
#include "actions.h"
#include "machines.h"

using namespace BSP;

cache_t cache;
void cacheinit(){
    memset(&cache, 0, sizeof(cache_t));
    cache.allok = 1;
    cache.voltok = 1;
    cache.tempok = 1;
    cache.commsok = 0;

    // mux settings... maximum values; expect rollover immediately
    cache.mux = 1;
    cache.muxpin = 7;
}

uint8_t tmux = 0;
uint8_t tpin = 6;

void masterdrive(void);
void slavedrive(void);
void leddrive(machine<uint32_t*>* m);

leddata led1 = {0, gpio::PortD, 2};
leddata led2 = {0, gpio::PortA, 3};

machine<masterstates_t> master(startup, masterdrive);
machine<slavestates_t> slave(off, slavedrive);
machine<uint32_t*> ledok(ledstates[1], NULL, leddrive, (void*)&led1);
machine<uint32_t*> ledstatus(ledstates[0], NULL, leddrive, (void*)&led2);

int main(void) {
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    SysTick_Config(SYSTICK);

    cacheinit();
    spiinit();
    actinit();

    master.setTimer(ms(500));
    master.start();
    ledok.start();
    ledstatus.start();

    while(1){
        actexec();
        master.run();
//        slave.run();
        ledok.run();
        ledstatus.run();
    }
/*

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

void masterdrive(void){

    switch(master.state){

        case startup:
            if(cache.commsok){
                setup();
                master.state = normalOk;
                break;
            }
            wakeup();
            bms::rdcfga();
            break;

        case temps:
            setup();
            wakeup();
            muxOff((tmux+1)%2);
            muxOn(tmux, tpin);
            master.state = hold;
            break;
        
        case normalOk:
            if(!cache.allok){
                master.setTimer(ms(4000));
                master.state = normalError;
                break;
            }
            wakeup();
            stepMux();
            //actwait(ms(20));
            //wakeup();
            readall();
            break;

        case normalError:
            panic();
            wakeup();
            readall();
            break;

        case hold:
            wakeup();
            bms::rdcfga();
            break;

        default:
            break;

    }

}

void slavedrive(void){

}

void leddrive(machine<uint32_t*>* m){
    leddata* data = (leddata*)m->data;
    if(m->state[data->counter] == 0){
        data->counter = 0;
        gpio::GPIO::set(data->port, data->pin);
    } else {
        gpio::GPIO::toggle(data->port, data->pin);
    }
    m->setTimer(m->state[data->counter]);
    data->counter++;
}

extern "C" {
void SysTick_Handler(void){
    acttick();
    master.tick();
    slave.tick();
    ledok.tick();
    ledstatus.tick();
}
}
