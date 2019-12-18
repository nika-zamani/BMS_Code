#ifndef COMMANDSEND_H
#define COMMANDSEND_H

#include "common.h"
#include "bmsCommand.h"
#include "transaction.h"

// commandRDCFGA: tests read of config reg group A.
// should read F2-00-00-00-00-00 or similar.
// to include test task, add to main:
// xTaskCreate(commandRDCFGA, "rdcfgatest", STACK_SIZE, NULL, 1, NULL );
void commandRDCFGA(void *pvParameters);

// commandWRCFGA: tests write to config reg group A. Should set S0
// to include test task, add to main:
// xTaskCreate(commandWRCFGA, "wrcfgatest", STACK_SIZE, NULL, 1, NULL );
void commandWRCFGA(void *pvParameters);
void commandRW(void *pvParameters);

#endif // COMMANDSEND_H


