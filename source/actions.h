#ifndef ACTIONS_H_
#define ACTIONS_H_

#include <stdint.h>
#include "ltcutility.h"
#include "gpio.h"
#include "defines.h"
#define qsize 200 // total action queue size

enum actions_t {
    wait,
    csdown,
    csup,
    spitx,
    spiwait,
    execute
};

void actwait(uint32_t x);
void actcsdown();
void actcsup();
void actspitx(bms::bmscommands_t comm, uint8_t* data, uint32_t len);
void actspiwait();
void actexecute(void (*)(void));

struct action_t {
    actions_t a;
    uint32_t wait;
    uint32_t len;
    uint8_t data[4+(8*slaves)];
    bms::bmscommands_t comm;
    void (*f)(void);
};

void actinit(void);
uint8_t actqcheck(uint8_t n);
uint8_t actaddsimple(actions_t action);
uint8_t actadd(action_t action);
uint8_t actinsert(action_t action);
uint8_t actexec(void);
void acttick(void);

#endif
