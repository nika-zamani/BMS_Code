#ifndef ACTIONS_H_
#define ACTIONS_H_

#include <stdint.h>
#include "ltcutility.h"
#include "gpio.h"
#include "defines.h"
#define qsize 10

enum actions_t {
    wait,
    csdown,
    csup,
    spitx,
    spiwait
};

struct action_t {
    actions_t a;
    uint32_t wait;
    uint32_t len;
    uint8_t data[4+(8*slaves)];
    bms::bmscommands_t comm;
};

struct action_t actions[qsize];
volatile uint8_t qhead;
volatile uint8_t qtail;
volatile uint32_t qwait;

void actinit(void);
uint8_t actqcheck(uint8_t n);
uint8_t actadd(action_t action);
uint8_t actexec(void);
void acttick(void);

#endif
