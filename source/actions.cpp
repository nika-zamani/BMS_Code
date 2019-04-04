#include "actions.h"

void actinit(void){
    qhead = 0;
    qtail = 0;
    qwait = 0;
}

void actwait(uint32_t x){
    action_t a;
    a.a = wait;
    a.wait = x;
    actadd(a);
}

void actspitx(bms::bmscommands_t comm, uint8_t* data, uint32_t len){
    action_t a;
    a.a = spitx;
    a.comm = comm;
    memcpy(a.data, data, len);
    a.len = len;
    actadd(a);
}

void actspiwait(){
    action_t a; 
    a.a = spiwait;
    actadd(a);
}

uint8_t actqcheck(uint8_t n){
    // check to see if queue full
    uint8_t qct = qtail >= qhead ? qtail-qhead : (qsize-1)-qhead+qtail;
    if(qct+n > (qsize-1)) return 1;
    return 0;
}

uint8_t actadd(action_t action){
    actions[qtail] = action;
    qtail = (qtail+1)%qsize;
    return 0;
}

// insert action before others
uint8_t actinsert(action_t action){
    // temporary qhead marker for interrupt safety
    uint8_t qheadtmp = (qhead-1)%qsize;
    actions[qheadtmp] = action;
    qhead = qheadtmp;
    return 0;
}

void acttick(void){
    if(qwait) qwait--;
}

uint8_t actexec(void){
    if(!qwait && qtail != qhead) {
        action_t a = actions[qhead];
        
        switch(a.a){
            case wait:
                qwait = a.wait;
                break;

            case csdown:
                BSP::gpio::GPIO::clear(ltccsport, ltccspin);
                break;

            case csup:
                BSP::gpio::GPIO::set(ltccsport, ltccspin);
                break;

            case spitx:
                bms::transmit(a.data, a.len, a.comm);
                break;

            // wait for spi transaction to finish
            case spiwait:
                if(bms::spistatus()) return 0;
                break;

            default:
                break;
        }
        qhead = (qhead+1)%qsize;
    }
    return 0;
}
