#include "bms.h"

BMS::BMS() 
{
    input.current_adc = 0;
    input.max_temp = 0;
    input.sum_voltage = 0;

    input.ts_live = false;
    input.ts_ready = false;
    input.dcdc_fault = false;
    input.dcdc_temp = 0;

    input.is_charging = false;

    input.lowest_volt = 0;

    output.bms_ok = true;
    output.airs_positive = DIGITAL_LOW;
    output.airs_negative = DIGITAL_LOW;
    output.precharge = DIGITAL_LOW;
    output.dcdc_enable = DIGITAL_LOW;

    
};