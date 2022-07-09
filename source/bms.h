#ifndef BMS_H
#define BMS_H

#include "MKE18F16.h"
#include "common.h"

enum DIGITAL
{
    DIGITAL_LOW,
    DIGITAL_HIGH,
};

typedef struct 
{

    uint16_t cell_voltages[SLAVE_COUNT][12];
    uint16_t thermistor_values[SLAVE_COUNT][16];

    uint16_t current_adc;
    uint16_t max_temp;
    uint32_t sum_voltage;

    uint8_t ts_live;
    uint8_t ts_ready;
    uint8_t dcdc_fault;
    uint16_t dcdc_temp;

    uint8_t is_charging;

    uint16_t lowest_volt;

} input_t;

typedef struct 
{

    uint8_t bms_ok;
    uint8_t airs_positive;
    uint8_t airs_negative;
    uint8_t precharge;
    uint8_t dcdc_enable;

} output_t;

class BMS 
{
    public:
        volatile input_t input;
        volatile output_t output;

    BMS();

};

#endif