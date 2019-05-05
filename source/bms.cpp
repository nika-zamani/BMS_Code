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
#include "drivers.h"
#include "cache.h"
#include "actions.h"
#include "machines.h"

using namespace BSP;

/* machine state */
cache_t cache;
void cacheinit(){
    memset(&cache, 0, sizeof(cache_t));
    cache.allok = 0;
    cache.voltok = 0;   // cleared until proven good
    cache.tempok = 0;   // just assert until temp logic is finished
    cache.commsok = 0;  // asserts on receiving all good PEC

    cache.reset = 1;    // 1 = resetting once after error is OK

    // preload temperatures array
    for(uint8_t i = 0; i < slaves; i++)
        cache.tempArray[i] = tempconfig;

    // mux settings... maximum values; expect rollover immediately
    cache.mux = 1;
    cache.muxpin = 7;
}

/* all the submachines which control the flow */

void masterdrive(void);
machine<masterstates_t> master(link, masterdrive);


void leddrive(machine<uint32_t*>* m);

leddata led1 = {0, gpio::PortD, 2};
machine<uint32_t*> ledok(NULL, NULL, leddrive, (void*)&led1);

leddata led2 = {0, gpio::PortA, 3};
machine<uint32_t*> ledstatus(NULL, NULL, leddrive, (void*)&led2);


/* function to update the outside world */
void update();

int main(void) {
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    SysTick_Config(SYSTICK);

    bmsNotOkOut(); // just to be sure

    cacheinit();
    spiinit();
    caninit();
    actinit();

    master.setTimer(ms(250));
    master.start();
    ledok.start();
    ledstatus.start();

    while(1){
        update();
        actexec();
        master.run();
        ledok.run();
        ledstatus.run();
        if(cache.timeout.can == ms(500)){
            cache.timeout.can = 0;
            cantransmit();
        }
    }
}

void update(){

    // first, clear bmsok if necessary
    if(!cache.allok){
        bmsNotOkOut();
    } else {
    // set bmsok ONLY if resetting is permitted
        if(cache.reset && master.state == normalOk){
            cache.reset = 0;
            bmsOkOut();
        }
    }

    // second, update LED behaviors
    switch(master.state){
        case link:
            ledok.state = ledstates[2];
            ledstatus.state = ledstates[2];
            break;

        case startup:
            ledok.state = ledstates[5];
            ledstatus.state = ledstates[4];
            break;

        case normalOk:
            ledok.state = ledstates[0];
            ledstatus.state = NULL;
            break;

        case normalError:
            ledok.state = NULL;
            ledstatus.state = ledstates[3];
            break;

        default:
            break;
    }

}


void masterdrive(void){

    switch(master.state){

        case link:
            if(cache.commsok){
                setup();
                master.state = startup;
                break;
            }
            wakeup();
            bms::rdcfga();
            break;

        /* deprecated test code
        case temps:
            setup();
            wakeup();
            muxOff((tmux+1)%2);
            muxOn(tmux, tpin);
            master.state = hold;
            break;
            */

        case startup:
            if(cache.allok){
                master.state = normalOk;
                break;
            }
            wakeup();
            stepMux();
            readall();
            break;
        
        case normalOk:
            if(!cache.allok){
                master.state = normalError;
                break;
            }
            wakeup();
            stepMux();
            readall();
            break;

        case normalError:
            wakeup();
            stepMux();
            readall();
            break;

        /* deprecated test code
        case hold:
            wakeup();
            bms::rdcfga();
            break;
            */

        default:
            break;

    }

}

void leddrive(machine<uint32_t*>* m){
    leddata* data = (leddata*)m->data;
    if(!m->state) {
        gpio::GPIO::clear(data->port, data->pin);
        return;
    }
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
    ledok.tick();
    ledstatus.tick();
    cache.timeout.tick();
}
}
