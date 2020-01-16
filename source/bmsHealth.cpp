#include "bmsHealth.h"

uint8_t RETURN_DATA[6];

uint16_t _CELLPU[12];
uint16_t _CELLPD[12];
uint16_t _CELLDELTA[12];

const uint8_t _MD = 2;  // filtered mode
const uint8_t _DCP = 0; // discharge not permited
const uint8_t _CH = 0;  // all cells
const _ADCOPT = 0; // the default value

int getSelfTestOutputPatern(uint8_t adcopt, uint8_t md, uint8_t st) {
    if (md == 1) {
        if (adcopt == 1) {
            return st == 1 ? 0x9565 : 0x6A9A;  
        }
        return st == 1 ? 0x9553 : 0x6AAC;
    }
    return st == 1 ? 0x9555 : 0x6AAA;
}

void monitorBMSHealth( void *pvParameters )
{
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = tick_ms(1000);

    // Initialise the xLastWakeTime variable with the current time.
    xLastWakeTime = xTaskGetTickCount();

    for (;;)
    {
        // perform diagnostic tests

        vTaskDelayUntil( &xLastWakeTime, xFrequency );
    }
}

int wiresOpen() {
    int open = 0;
    /*1. Run the 12-cell command ADOW with PUP = 1 at least
    twice. Read the cell voltages for cells 1 through 12 once
    at the end and store them in array CELLPU(n).*/
    int error = pushCommand(ADOW, SLAVE_COUNT, RETURN_DATA, _MD, 1 /*pup*/, _DCP, _CH);
    int error = pushCommand(ADOW, SLAVE_COUNT, RETURN_DATA, _MD, 1 /*pup*/, _DCP, _CH);

    int error = pushCommand(RDCVA, SLAVE_COUNT, RETURN_DATA);
    memcpy(_CELLPU, RETURN_DATA, 6);
    int error = pushCommand(RDCVB, SLAVE_COUNT, RETURN_DATA);
    memcpy(_CELLPU[6], RETURN_DATA, 6);
    int error = pushCommand(RDCVC, SLAVE_COUNT, RETURN_DATA);
    memcpy(_CELLPU[12], RETURN_DATA, 6);
    int error = pushCommand(RDCVD, SLAVE_COUNT, RETURN_DATA);
    memcpy(_CELLPU[18], RETURN_DATA, 6);


    /*2. Run the 12-cell command ADOW with PUP = 0 at least
    twice. Read the cell voltages for cells 1 through 12 once
    at the end and store them in array CELLPD(n).*/

    int error = pushCommand(ADOW, SLAVE_COUNT, RETURN_DATA, _MD, 0 /*pup*/, _DCP, _CH);
    int error = pushCommand(ADOW, SLAVE_COUNT, RETURN_DATA, _MD, 0 /*pup*/, _DCP, _CH);

    int error = pushCommand(RDCVA, SLAVE_COUNT, RETURN_DATA);
    memcpy(_CELLPD, RETURN_DATA, 6);
    int error = pushCommand(RDCVB, SLAVE_COUNT, RETURN_DATA);
    memcpy(_CELLPD[6], RETURN_DATA, 6);
    int error = pushCommand(RDCVC, SLAVE_COUNT, RETURN_DATA);
    memcpy(_CELLPD[12], RETURN_DATA, 6);
    int error = pushCommand(RDCVD, SLAVE_COUNT, RETURN_DATA);
    memcpy(_CELLPD[18], RETURN_DATA, 6);

    /*3. Take the difference between the pull-up and pull-down
    measurements made in above steps for cells 2 to 12:
    CELLΔ(n) = CELLPU(n) – CELLPD(n).*/
    for(int i = 0; i < 12; i++) {
        _CELLDELTA[i] = _CELLPU[i] - _CELLPD[i]
    }
    /*4. For all values of n from 1 to 11: If CELLΔ(n+1) < –400mV,
    then C(n) is open. If CELLPU(1) = 0.0000, then C(0) is
    open. If CELLPD(12) = 0.0000, then C(12) is open.*/
    for( int i = 1; i < SLAVE_COUNT; i++) {
        if(_CELLDELTA[i+1] < -400) {
            open++;
        }
    }
    if(_CELLPU[1] == 0) {
        open++;
    }
    if(_CELLPD[SLAVE_COUNT] == 0) {
        open++;
    }
    return open;
}

uint8_t selfTests(uint8_t md, uint8_t st) {
    //DIAGN
    int error = pushCommand(CVST, SLAVE_COUNT, RETURN_DATA, md, st);
    int error = pushCommand(AXST, SLAVE_COUNT, RETURN_DATA, md, st);
    int error = pushCommand(STATST, SLAVE_COUNT, RETURN_DATA, md, st);

    return checkSelfTest(md, st);
}

/* 
 * Checs the results of a self test and compares them to the expected results
 *      given a mode and self test mode
 * 
 *  @param md: the ADC mode to use in the check
 *  @param st: the self test mode to use in the check
 * 
 *  @return: a uint8_t with bits set corresponding to error codes fron each self test register group
 * 
 * Bits we check:
 * C1V - C12V (CVA, CVB, CVC, CVD): CVST Result 
 * G1V - G5V (AUXA, AUXB): AXST Result
 * SC, ITMP, VA, VD (STATA, STATB): STATST Result
*/
uint8_t checkSelfTest(uint8_t md, uint8_t st) {
    int expect = getSelfTestOutputPatern(_ADCOPT, md, st);

    uint8_t error = 0;

    int error = pushCommand(RDCVA, SLAVE_COUNT, RETURN_DATA);
    if (RETURN_DATA != expect) { error |= ERR_CVA; /* SELF TEST FAILED IN CVA */ }
    int error = pushCommand(RDCVB, SLAVE_COUNT, RETURN_DATA);
    if (RETURN_DATA != expect) { error |= ERR_CVB; /* SELF TEST FAILED IN CVB */ }
    int error = pushCommand(RDCVC, SLAVE_COUNT, RETURN_DATA);
    if (RETURN_DATA != expect) { error |= ERR_CVC; /* SELF TEST FAILED IN CVC */ }
    int error = pushCommand(RDCVD, SLAVE_COUNT, RETURN_DATA);
    if (RETURN_DATA != expect) { error |= ERR_CVD; /* SELF TEST FAILED IN CVD */ }

    int error = pushCommand(RDAUXA, SLAVE_COUNT, RETURN_DATA);
    if (RETURN_DATA != expect) { error |= ERR_AUXA; /* SELF TEST FAILED IN AUXA */ }
    int error = pushCommand(RDAUXB, SLAVE_COUNT, RETURN_DATA);
    if (RETURN_DATA != expect) { error |= ERR_AUXB; /* SELF TEST FAILED IN AUXB */ }

    int error = pushCommand(RDSTATA, SLAVE_COUNT, RETURN_DATA);
    if (RETURN_DATA != expect) { error |= ERR_STATA; /* SELF TEST FAILED IN STATA */ }
    int error = pushCommand(RDSTATB, SLAVE_COUNT, RETURN_DATA);
    if (RETURN_DATA != expect) { error |= ERR_STATB; /* SELF TEST FAILED IN STATB */ }

    return error;
}

/* Other diagnostic data we may want to check:
 *
 * MUXFAIL (STATB): Multiplexer self test result
 * THSD (STATB): Thermal Shutdown Status
*/