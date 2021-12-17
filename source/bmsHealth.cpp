#include "bmsHealth.h"
#include "gpio.h"
#include "can.h"
#include "CanMessage.h"
#include "adc.h"
#include "adcObject.h"
#include "main.h"
#include "common.h"

using namespace BSP;

uint8_t RETURN_DATA[6 * SLAVE_COUNT];
// uint16_t RETURN_DATA_TEMP[6 * SLAVE_COUNT];
// uint8_t RETURNED[6 * SLAVE_COUNT];
//uint8_t TEMP[15][6 * SLAVE_COUNT];
// 10 boards, either nested array or put into one whole

uint8_t BMS_OK = true; // signal for -- All cell votlages within limits, All cell temps within limits, No fuses open aka wires open (will be added later most likley)
// 0 is false, 1 is true; 
// need to output this to gpio
uint8_t IMD_OK = true;

uint16_t _CELLPU[12]; //TODO: check if number of cells will always be 12 and change this to a const
uint16_t _CELLPD[12];
int32_t _CELLDELTA[12];

// uint16_t _CELL_VOLTAGES[12];
uint16_t _CELL_VOLTAGES[SLAVE_COUNT][12];
// ten maybe sums
uint16_t _SUM_CELL_VOLTAGES = 0;


uint16_t _THERMISTOR_VALUES[SLAVE_COUNT][16];
uint16_t _THERM[SLAVE_COUNT][16];
uint16_t _THERMISTOR_CALCULATE = 0;
uint16_t _THERMISTOR_VOLTAGE = 0;
// uint16_t _THERMISTOR_VALUES[28];

uint8_t _SELF_TEST_FLAGS = 0;

const uint8_t _MD = 2;  // filtered mode
const uint8_t _ST = 1;  // self test 1
const uint8_t _DCP = 0; // discharge not permited
const uint8_t _CH = 0;  // all cells
const uint8_t _ADCOPT = 0; // the default value

uint16_t getSelfTestOutputPatern(uint8_t adcopt, uint8_t md, uint8_t st) {
    if (md == 1) {
        if (adcopt == 1) {
            return st == 1 ? 0x9565 : 0x6A9A;  
        }
        return st == 1 ? 0x9553 : 0x6AAC;
    }
    return st == 2 ? 0x9555 : 0x6AAA;
}

bool testPattern(uint8_t data[6], uint16_t pattern, uint8_t testLength = 6) {
    for (uint8_t i = 0; i < testLength; i+=2)
    {
        if(data[i] != (uint8_t)(pattern) || data[i+1] != (uint8_t)(pattern>>8)) {
            return false;
        }
    }
    return true;
}

// int wiresOpen() {
//     int open = 0;
//     int error = 0;
//     /*1. Run the 12-cell command ADOW with PUP = 1 at least
//     twice. Read the cell voltages for cells 1 through 12 once
//     at the end and store them in array CELLPU(n).*/
//     for(int i = 0; i < 100; i++) {
//         error = pushCommand(ADOW, SLAVE_COUNT, RETURN_DATA, 3, 1 /*pup*/, _DCP, _CH);
//     }

//     error = pushCommand(RDCVA, SLAVE_COUNT, RETURN_DATA);
//     memcpy(&_CELLPU, RETURN_DATA, 6);
//     error = pushCommand(RDCVB, SLAVE_COUNT, RETURN_DATA);
//     memcpy(&_CELLPU[3], RETURN_DATA, 6);
//     error = pushCommand(RDCVC, SLAVE_COUNT, RETURN_DATA);
//     memcpy(&_CELLPU[6], RETURN_DATA, 6);
//     error = pushCommand(RDCVD, SLAVE_COUNT, RETURN_DATA);
//     memcpy(&_CELLPU[9], RETURN_DATA, 6);


//     /*2. Run the 12-cell command ADOW with PUP = 0 at least
//     twice. Read the cell voltages for cells 1 through 12 once
//     at the end and store them in array CELLPD(n).*/

//     for(int i = 0; i < 100; i++) {
//         error = pushCommand(ADOW, SLAVE_COUNT, RETURN_DATA, 3, 0 /*pup*/, _DCP, _CH);
//     }


//     error = pushCommand(RDCVA, SLAVE_COUNT, RETURN_DATA);
//     memcpy(&_CELLPD, RETURN_DATA, 6);
//     error = pushCommand(RDCVB, SLAVE_COUNT, RETURN_DATA);
//     memcpy(&_CELLPD[3], RETURN_DATA, 6);
//     error = pushCommand(RDCVC, SLAVE_COUNT, RETURN_DATA);
//     memcpy(&_CELLPD[6], RETURN_DATA, 6);
//     error = pushCommand(RDCVD, SLAVE_COUNT, RETURN_DATA);
//     memcpy(&_CELLPD[9], RETURN_DATA, 6);

//     /*3. Take the difference between the pull-up and pull-down
//     measurements made in above steps for cells 2 to 12:
//     CELLΔ(n) = CELLPU(n) – CELLPD(n).*/
//     for(int i = 0; i < 150; i++) {
//         _CELLDELTA[i] = _CELLPU[i] - _CELLPD[i];
//     }
//     /*4. For all values of n from 1 to 11: If CELLΔ(n+1) < –400mV,
//     then C(n) is open. If CELLPU(1) = 0.0000, then C(0) is
//     open. If CELLPD(12) = 0.0000, then C(12) is open.*/
//     for( int i = 1; i < 11; i++) {
//         if(_CELLDELTA[i+1] < -400) {
//             open++;
//         }
//     }
//     if(_CELLPU[0] == 0) {
//         open++;
//     }
//     if(_CELLPD[11] == 0) {
//         open++;
//     }
//     return open;
// }

// /* 
//  * Checks the results of a self test and compares them to the expected results
//  *      given a mode and self test mode
//  * 
//  *  @param md: the ADC mode to use in the check
//  *  @param st: the self test mode to use in the check
//  * 
//  *  @return: a uint8_t with bits set corresponding to error codes fron each self test register group
//  * 
//  * Bits we check:
//  * C1V - C12V (CVA, CVB, CVC, CVD): CVST Result 
//  * G1V - G5V (AUXA, AUXB): AXST Result
//  * SC, ITMP, VA, VD (STATA, STATB): STATST Result
// */
// uint8_t selfTest(uint8_t md, uint8_t st) {
//     int expect = getSelfTestOutputPatern(_ADCOPT, md, st);
//     int error = 0;
//     uint8_t error_flags = 0;

//     memset(RETURN_DATA, 0, 6);

//     error = pushCommand(CVST, SLAVE_COUNT, RETURN_DATA, md, st);

//     error = pushCommand(RDCVA, SLAVE_COUNT, RETURN_DATA);
//     if (!testPattern(RETURN_DATA,expect)) { error_flags |= ERR_CVA; /* SELF TEST FAILED IN CVA */ }
//     error = pushCommand(RDCVB, SLAVE_COUNT, RETURN_DATA);
//     if (!testPattern(RETURN_DATA,expect)) { error_flags |= ERR_CVB; /* SELF TEST FAILED IN CVB */ }
//     error = pushCommand(RDCVC, SLAVE_COUNT, RETURN_DATA);
//     if (!testPattern(RETURN_DATA,expect)) { error_flags |= ERR_CVC; /* SELF TEST FAILED IN CVC */ }
//     error = pushCommand(RDCVD, SLAVE_COUNT, RETURN_DATA);
//     if (!testPattern(RETURN_DATA,expect)) { error_flags |= ERR_CVD; /* SELF TEST FAILED IN CVD */ }

//     error = pushCommand(AXST, SLAVE_COUNT, RETURN_DATA, md, st);

//     error = pushCommand(RDAUXA, SLAVE_COUNT, RETURN_DATA);
//     if (!testPattern(RETURN_DATA,expect)) { error_flags |= ERR_AUXA; /* SELF TEST FAILED IN AUXA */ }
//     error = pushCommand(RDAUXB, SLAVE_COUNT, RETURN_DATA);
//     if (!testPattern(RETURN_DATA,expect)) { error_flags |= ERR_AUXB; /* SELF TEST FAILED IN AUXB */ }

//     error = pushCommand(STATST, SLAVE_COUNT, RETURN_DATA, md, st);

//     error = pushCommand(RDSTATA, SLAVE_COUNT, RETURN_DATA);
//     if (!testPattern(RETURN_DATA,expect)) { error_flags |= ERR_STATA; /* SELF TEST FAILED IN STATA */ }
//     error = pushCommand(RDSTATB, SLAVE_COUNT, RETURN_DATA);
//     if (!testPattern(RETURN_DATA,expect, 2)) { error_flags |= ERR_STATB; /* SELF TEST FAILED IN STATB */ }

//     return error_flags;
// }

/* Other diagnostic data we may want to check:
 *
 * MUXFAIL (STATB): Multiplexer self test result
 * THSD (STATB): Thermal Shutdown Status
*/

/*  Gets the voltages for each cell and stores them in _CELL_VOLTAGES and the sum of cell voltages in _SUM_CELL_VOLTAGES
 * 
 *  @param md: the mode to get the voltages with
 */
void getVoltages(uint8_t md) {
    int error = 0;
    memset(RETURN_DATA, 0, sizeof(RETURN_DATA));
    pushCommand(ADCVSC, SLAVE_COUNT, RETURN_DATA, md, _DCP);

    // wait for some time?
    // SLAVE_COUNT constant

    error = pushCommand(RDCVA, SLAVE_COUNT, RETURN_DATA);
    for (int i = 0; i < SLAVE_COUNT; i++) {
        memcpy(&_CELL_VOLTAGES[i], (RETURN_DATA + (i * 6)), 6); 
    }
    // return_data is array size 6 of uint8_t and each cell_voltage is array size of 12 of uint16_t
    error = pushCommand(RDCVB, SLAVE_COUNT, RETURN_DATA);
    for (int i = 0; i < SLAVE_COUNT; i++) {
        memcpy(&_CELL_VOLTAGES[i][3], (RETURN_DATA + (i * 6)), 6);
    }
    // so each return_data takes up 3 spaces of 12 in cell_voltage
    error = pushCommand(RDCVC, SLAVE_COUNT, RETURN_DATA);
    for (int i = 0; i < SLAVE_COUNT; i++) {
        memcpy(&_CELL_VOLTAGES[i][6], (RETURN_DATA + (i * 6)), 6);
    }
    error = pushCommand(RDCVD, SLAVE_COUNT, RETURN_DATA);
    // memcpy(&_CELL_VOLTAGES[9], RETURN_DATA, 6);    


    error = pushCommand(RDSTATA, SLAVE_COUNT, RETURN_DATA);

}

/*  Gets and returns the raw tempurature for each thermistor
 */
void getTempuraturesHelper() {
    int error = 0;
    memset(RETURN_DATA, 0, sizeof(RETURN_DATA));
    int i = 0;
    

    for (int j = 0; j < 8; j++) {
        
        muxSet(0, 7);
        muxSet(1, 7);
        error = pushCommand(ADCVAX, SLAVE_COUNT, RETURN_DATA, 1);
        vTaskDelay(50);
        error = pushCommand(RDAUXA, SLAVE_COUNT, RETURN_DATA);
        
    
        // when it errors dont push data; if result hasnt been sent in a while, throw a fault
        for (int i = 0; i < SLAVE_COUNT; i++) {
        
            _THERMISTOR_VALUES[i][2*j] = RETURN_DATA[i*6] | RETURN_DATA[i*6+1]<<8;   // add 4 when it is collected; store raw data for now
            _THERMISTOR_VALUES[i][2*j + 1] = RETURN_DATA[i*6+2] | (RETURN_DATA[i*6+3]<<8);


        }
    }
    // }
}

void readIMD_OK() {
    gpio::GPIO::StaticClass().read(gpio::PortD, 15);
}

void calculateBMS_OK() {
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 8; j++) {
            // change later maybe for rows of cells
            if (_CELL_VOLTAGES[i][j] < 28000 | _CELL_VOLTAGES[i][j] > 50000) {    // lower limit voltage = 2.8V for now
                BMS_OK = false;
                gpio::GPIO::StaticClass().clear(gpio::PortD, 15); // port, pin -- tbd need to ask for later
                // put pin in a header file, constants.h/pins.h 
            }
        }
    }

    // imd is gpio out - true false gpio signal
    // false when things are bad, true when things are good

    // memset 0 for the bytes in can struct

    // 0 C min, 70 C max for now for thermistor limits
    // rules say max temp 60 C
}

void measureSendVoltageCurrent() {
    adcObject *voltageMeasure = new adcObject(VOLTAGE_ADC, VOLTAGE_ADC_CHANNEL);
    adcObject *currentMeasure = new adcObject(CURRENT_ADC, CURRENT_ADC_CHANNEL);
    
    voltageMeasure->adcRead();
    currentMeasure->adcRead();

    CanMessage::sendMainVoltageCurrent(voltageMeasure->adc_data, currentMeasure->adc_data);

}

void monitorBMSHealth( void *pvParameters )
{
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = tick_ms(1000);

    uint8_t testResults;
    uint8_t wireOpen;

    // Initialise the xLastWakeTime variable with the current time.
    xLastWakeTime = xTaskGetTickCount();


    uint8_t confdat[6*SLAVE_COUNT];
    memset(confdat, 0, 6*SLAVE_COUNT);
    for(uint8_t i = 0; i < SLAVE_COUNT; i++) confdat[6*i] = 0xFC;
    pushCommand(WRCFGA, SLAVE_COUNT, confdat);

    for (;;)
    {
        // perform diagnostic tests

        // getVoltages(1);
        // for (int id = 0; id < SLAVE_COUNT; id++) {
        //     CanMessage::sendVoltage(_CELL_VOLTAGES[id], id);    // 10 Hz
        // }

        getTempuraturesHelper();
        // for (int i = 0; i < SLAVE_COUNT; i++) {
        //     getTempuratures(i + 1, i);
        // }

        // for (int id = 0; id < 10; id++) {
        //     CanMessage::sendTemp(_THERMISTOR_VALUES[id], id);   // 15 Hz
        // }

        // calculate BMS-OK
        // calculateBMS_OK();
        // readIMD_OK();
        // CanMessage::sendImdBmsOk(BMS_OK, IMD_OK);   // send bms ok and imd ok

        // get and send main voltage and current 
        // measureSendVoltageCurrent();

        vTaskDelayUntil( &xLastWakeTime, xFrequency );
    }
}
