#ifndef BMSCOMMAND_H
#define BMSCOMMAND_H

#include "MKE18F16.h"

// Array contains command codes for all commands neccecary, position in this array will depend
//      on a mapped value from the command name in bms.h
const uint8_t CCS[2][2] = {
    {0x00, 0x1E}, //RDSPSB*
    {0x00, 0x0F}  //STSCTRL
};

// a command to be sent to the BMS
typedef struct bmscommand_t {
    int command;            // integer representation of a command
    int size;               // size of the data
    int num;                // number of chips
    uint8_t **data;   // pointer to 2D array of data for each chip
} bmscommand_t;

void bmsCommandInit(bmscommand_t *c, int com, int length, int num, uint8_t **data);

int buildCommandBuffer(bmscommand_t *command, uint8_t *tx);

int bmsCommandSize(bmscommand_t *c);

#endif