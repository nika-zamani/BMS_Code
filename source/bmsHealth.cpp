#include "bmsHealth.h"

uint8_t RETURN_DATA[6];

uint16_t _CELLPU[12];
uint16_t _CELLPD[12];
uint16_t _CELLDELTA[12];

const uint8_t _MD = 2;  // filtered mode
const uint8_t _DCP = 0; // discharge not permited
const uint8_t _CH = 0;  // all cells

void commandRDCFGA( void *pvParameters )
{
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = tick_ms(1000);

    // Initialise the xLastWakeTime variable with the current time.
    xLastWakeTime = xTaskGetTickCount();

    for (;;)
    {
        int error = pushCommand(RDCFGA, SLAVE_COUNT, RETURN_DATA);

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

void startSelfTests(uint8_t md, uint8_t st) {
    //DIAGN
    int error = pushCommand(CVST, SLAVE_COUNT, RETURN_DATA, md, st);
    int error = pushCommand(AXST, SLAVE_COUNT, RETURN_DATA, md, st);
    int error = pushCommand(STATST, SLAVE_COUNT, RETURN_DATA, md, st);

}