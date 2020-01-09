#ifndef BMSHEALTH_H
#define BMSHEALTH_H

#include "common.h"
#include "bmsCommand.h"
#include "transaction.h"

// commandRDCFGA: tests read of config reg group A.
// should read F2-00-00-00-00-00 or similar.
// to include test task, add to main:
// xTaskCreate(commandRDCFGA, "rdcfgatest", STACK_SIZE, NULL, 1, NULL );
void commandRDCFGA(void *pvParameters);

#endif // BMSHEALTH_H


