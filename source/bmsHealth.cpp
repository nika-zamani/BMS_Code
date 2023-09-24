#include "bmsHealth.h"

uint8_t RETURN_DATA[6 * SLAVE_COUNT];
uint8_t SCONTROL_DATA[6 * SLAVE_COUNT];
uint8_t DCC_DATA[6 * SLAVE_COUNT];
const uint16_t CALIBRATED_REF_VOLTAGES[28800, 26000, 28200, 29600, 30000]
const uint8_t _DCP = 0; // discharge not permited

extern BMS bms;

uint32_t calcTempToVolt(uint16_t temperature)
{
    uint32_t v1 = (-0.0000040337 * temperature * temperature * temperature * temperature) 
                  + (0.01131 * temperature * temperature * temperature) 
                  - (1.32809 * temperature * temperature) 
                  - (166.1785 * temperature) 
                  + 23486.0113;
    return v1;
}

uint16_t calcVoltToTemp(uint16_t voltage)
{
    float temp = 156.91
                 + (-0.01188 * voltage)
                 + (0.0000002315 * voltage * voltage)
                 + (0.000000000010389 * voltage * voltage * voltage)
                 + (-0.000000000000000409 * voltage * voltage * voltage * voltage);

    return temp * 1000;
}

uint16_t calcVoltToResistance(uint16_t voltage, uint16_t refVoltage)
{
    resistance = ((-voltage)*DIVIDER_RESISTANCE)/(voltage-refvoltage);
    return resistance
}

uint16_t getMaxTemp()
{
    uint16_t tempMax = 0; 
    for (int i = 0; i < SLAVE_COUNT; i++)
    {
        tempMax = calcVoltToResistance(bms.input.thermistor_values[i][0], CALIBRATED_REF_VOLTAGES[i]);
        bms.input.thermistor_resistances[i][0] = tempMax;
        for (int j = 0; j < THERMISTOR_COUNT; j++)
        {
            bms.input.thermistor_resistances[i][j] = calcVoltToResistance(bms.input.thermistor_values[i][j], CALIBRATED_REF_VOLTAGES[i]);
            if (j >= THERMISTOR_COUNT) {
            }
            else {
                if (tempMax > bms.input.thermistor_resistances[i][j])
                {
                    tempMax = bms.input.thermistor_resistances[i][j];
                }
            }
        }
    }
    return tempMax;
}

uint32_t getSumVoltage()
{
    uint32_t sumVoltage = 0;
    uint16_t lowest_volt = 44000;
    uint16_t highest_volt = 0;
    for (int i = 0; i < SLAVE_COUNT; i++)
    {
        for (int j = 0; j < CELL_COUNT; j++)
        {
            sumVoltage += bms.input.cell_voltages[i][j];
            if (lowest_volt > bms.input.cell_voltages[i][j]) {
                lowest_volt = bms.input.cell_voltages[i][j]; }
            if (highest_volt < bms.input.cell_voltages[i][j]) {
                highest_volt = bms.input.cell_voltages[i][j]; }
        }
    }
    bms.input.lowest_volt = lowest_volt;
    bms.input.highest_volt = highest_volt;
    return sumVoltage;
}

void getVoltages(uint8_t md)
{
    int error = 0;
    memset(RETURN_DATA, 0, sizeof(RETURN_DATA));
    error = pushCommand(CLRSCTRL, SLAVE_COUNT, RETURN_DATA);

    pushCommand(ADCVSC, SLAVE_COUNT, RETURN_DATA, md, _DCP);

    // Push read ADCs command for each segment (A-F) and store cell voltages
    // RETURN_DATA => [Pack 1: {cell1, cell2, cell3} ,Pack 2: {cell1, cell2, cell3}...]
    //                [Bytes:   0-1,   2-3,   4-5,             6-7,   8-9,   10-11    ]

    error = pushCommand(RDCVA, SLAVE_COUNT, RETURN_DATA);
    for (int i = 0; i < SLAVE_COUNT; i++)
    {
        memcpy((void *)&bms.input.cell_voltages[i][0], (RETURN_DATA + (i * 6)), 6);
    }
    // return_data is array size 6 of uint8_t and each cell_voltage is array size of 12 of uint16_t
    error = pushCommand(RDCVB, SLAVE_COUNT, RETURN_DATA);
    for (int i = 0; i < SLAVE_COUNT; i++)
    {
        memcpy((void *)&bms.input.cell_voltages[i][3], (RETURN_DATA + (i * 6)), 6);
    }
    // so each return_data takes up 3 spaces of 12 in cell_voltage
    error = pushCommand(RDCVC, SLAVE_COUNT, RETURN_DATA);
    for (int i = 0; i < SLAVE_COUNT; i++)
    {
        memcpy((void *)&bms.input.cell_voltages[i][6], (RETURN_DATA + (i * 6)), 6);
    }

    error = pushCommand(RDCVD, SLAVE_COUNT, RETURN_DATA);
    for (int i = 0; i < SLAVE_COUNT; i++)
    {
        memcpy((void *)&bms.input.cell_voltages[i][9], (RETURN_DATA + (i * 6)), 6);
    }

    error = pushCommand(RDCVE, SLAVE_COUNT, RETURN_DATA);
    for (int i = 0; i < SLAVE_COUNT; i++)
    {
        memcpy((void *)&bms.input.cell_voltages[i][12], (RETURN_DATA + (i * 6)), 6);
    }

    error = pushCommand(RDCVF, SLAVE_COUNT, RETURN_DATA);
    for (int i = 0; i < SLAVE_COUNT; i++)
    {
        memcpy((void *)&bms.input.cell_voltages[i][15], (RETURN_DATA + (i * 6)), 2);
    }
    error = pushCommand(RDCVD, SLAVE_COUNT, RETURN_DATA);

    error = pushCommand(RDSTATA, SLAVE_COUNT, RETURN_DATA);
}

void SControl()
{
    // if (!bms.output.bms_ok){
    //     pushCommand(RDCFGA, SLAVE_COUNT, DCC_DATA);

    //     for (int i = 0; i < SLAVE_COUNT; i++)
    //     {
    //         // Clear DCC values 
    //         DCC_DATA[(i * 6) + 4] = 0;
    //     }

    //     // Push DCC changes
    //     pushCommand(WRCFGA, SLAVE_COUNT, DCC_DATA);

    //     return;
    // }

    int error = 0;
    memset(DCC_DATA, 0, sizeof(DCC_DATA));

    uint8_t discharge_channels[SLAVE_COUNT];

    // // Clears control
    // error = pushCommand(CLRSCTRL, SLAVE_COUNT, RETURN_DATA);
    // // ??
    error = pushCommand(ADCVSC, SLAVE_COUNT, RETURN_DATA, 1, _DCP); // is this needed?

    // SCONTROL_DATA[0] = 0b10000000;
    // SCONTROL_DATA[1] = 0b00000000;
    // SCONTROL_DATA[2] = 0b00000000;
    // SCONTROL_DATA[3] = 0b00000000;
    // SCONTROL_DATA[4] = 0b00000000;
    // SCONTROL_DATA[5] = 0b00000000;

    // Read config reg
    error = pushCommand(RDCFGA, SLAVE_COUNT, DCC_DATA);

    u_int16_t lowest = 0;
    for (int i = 0; i < SLAVE_COUNT; i++/*i=i+2*/)
    {
        lowest = 0xFFFF;

        // Find lowest voltage out of 8
        for (int j = 0; j < CELL_COUNT; j++)
        {
            if (bms.input.cell_voltages[i][j] < lowest){
                lowest = bms.input.cell_voltages[i][j];
            }

            // if (bms.input.cell_voltages[i+1][j] < lowest){
            //     lowest = bms.input.cell_voltages[i+1][j];
            // }
           
        }

        // Clear DCC values 
        DCC_DATA[((SLAVE_COUNT-1-i) * 6) + 4] = 0;
        DCC_DATA[((SLAVE_COUNT-1-(i+1)) * 6) + 4] = 0;

        // Set cells not withing .1 volts to discharge
        for (int j = 0; j < CELL_COUNT; j++)
        {
            // Check which ones off by .1 volt
            if ((bms.input.cell_voltages[i][j] - lowest) > DISHARGE_THRESHOLD){
                // If so discharge
                uint8_t tempaa = ((SLAVE_COUNT-1-i) * 6) + 4;
                DCC_DATA[((SLAVE_COUNT-1-i) * 6) + 4] |= (1 << j);
            }
            // Check which ones off by .1 volt
            // if ((bms.input.cell_voltages[i+1][j] - lowest) > DISHARGE_THRESHOLD){
            //     // If so discharge
            //     uint8_t tempaa = ((SLAVE_COUNT-1-(i+1)) * 6) + 4;
            //     DCC_DATA[((SLAVE_COUNT-1-(i+1)) * 6) + 4] |= (1 << j);
            // }
        }
        
    }

    // Push DCC changes
    DCC_DATA[0] |= 0xF8;
    DCC_DATA[6] |= 0xF8;
    DCC_DATA[12] |= 0xF8;
    DCC_DATA[18] |= 0xF8;
    DCC_DATA[24] |= 0xF8;
    error = pushCommand(WRCFGA, SLAVE_COUNT, DCC_DATA);
    // error = pushCommand(RDCFGA, SLAVE_COUNT, DCC_DATA);
    
    
    // // "Start S Control Pulsing and Poll Status"
    // pushCommand(STSCTRL, SLAVE_COUNT, RETURN_DATA); // put this here?????
    // for (int i = 0; i < 6; i++)
    // {
    //     uint8_t temp = SCONTROL_DATA[5];
    //     SCONTROL_DATA[5] = SCONTROL_DATA[4];
    //     SCONTROL_DATA[4] = SCONTROL_DATA[3];
    //     SCONTROL_DATA[3] = SCONTROL_DATA[2];
    //     SCONTROL_DATA[2] = SCONTROL_DATA[1];
    //     SCONTROL_DATA[1] = SCONTROL_DATA[0];
    //     SCONTROL_DATA[0] = temp;
    //     memset(RETURN_DATA, 0, sizeof(RETURN_DATA));
    //     error = pushCommand(WRSCTRL, SLAVE_COUNT, SCONTROL_DATA);
    //     vTaskDelay(100); // check ticks
    //     error = pushCommand(RDSCTRL, SLAVE_COUNT, RETURN_DATA);
    // }
    // pushCommand(STSCTRL, SLAVE_COUNT, RETURN_DATA);

    // memset(RETURN_DATA, 0, sizeof(RETURN_DATA));
    // error = pushCommand(WRSCTRL, SLAVE_COUNT, SCONTROL_DATA);
    // error = pushCommand(STSCTRL, SLAVE_COUNT, RETURN_DATA);

    // vTaskDelay(100);
    // error = pushCommand(RDSCTRL, SLAVE_COUNT, RETURN_DATA);
}

void getTempuratures(uint8_t md)
{
    int error = 0;
    memset(RETURN_DATA, 0, sizeof(RETURN_DATA));

    for (int j = 0; j < 8; j++)
    {
        muxSet(0, j);
        muxSet(1, j);

        error = pushCommand(ADAX, SLAVE_COUNT, RETURN_DATA, md, 0);
        vTaskDelay(100);
        error = pushCommand(RDAUXA, SLAVE_COUNT, RETURN_DATA);

        for (int i = 0; i < SLAVE_COUNT; i++)
        {
            bms.input.thermistor_values[i][j] = RETURN_DATA[i * 6] | RETURN_DATA[i * 6 + 1] << 8; // add 4 when it is collected; store raw data for now
            bms.input.thermistor_values[i][j + 8] = RETURN_DATA[i * 6 + 2] | (RETURN_DATA[i * 6 + 3] << 8);
        }
    }
}

void calculateBMS_OK()
{
    static uint64_t bms_start_time = xTaskGetTickCount();
    uint8_t bms_flag = true;
    for (int i = 0; i < SLAVE_COUNT; i++)
    {
        for (int j = 0; j < CELL_COUNT; j++)
        {
                if ((bms.input.cell_voltages[i][j] < BATTERY_VOLT_LIMIT_LOWER) | (bms.input.cell_voltages[i][j] > BATTERY_VOLT_LIMIT_HIGHER))
                {
                    bms_flag = false;
                    break;
                }
            if (j >= THERMISTOR_COUNT || (i == 1 && j == 9)) {
            }
            else {
                if (bms.input.thermistor_resistances[i][j] < THERMISTOR_RESISTANCE_LIMIT)
                {
                    bms_flag = false;
                    break;
                }
            }
        }
    }
    if (bms_flag) {
        bms_start_time = xTaskGetTickCount();
        bms.output.bms_ok = true;
    }
    else if (xTaskGetTickCount() - bms_start_time > ERROR_WAIT_TIME) {
        bms.output.bms_ok = false;
    }
}

void measureSendVoltageTempCurrent()
{
    bms.input.sum_voltage = getSumVoltage();
    bms.input.max_temp = getMaxTemp();
    bms.input.current_adc = measureCurrent();
    sendMainVoltageTempCurrent(bms.input.sum_voltage, bms.input.max_temp, bms.input.current_adc);
}

void sendVoltages()
{
    // Send all volts
    for (int id = 0; id < SLAVE_COUNT; id++)
    {
        sendVoltage((uint16_t *)&bms.input.cell_voltages[id], id);
        vTaskDelay(pdMS_TO_TICKS(1));
    }

    uint16_t lowest_volt = 44000;
    // Lowest volt
    for (int i = 0; i < SLAVE_COUNT; i++)
    {
        for (int j = 0; j < CELL_COUNT; j++)
        {
                if (bms.input.cell_voltages[i][j] < lowest_volt)
                {
                    lowest_volt = bms.input.cell_voltages[i][j];
                }
        }
    }
    
    sendLowestVolt(lowest_volt);
}

void sendTemperatures()
{
    for (int id = 0; id < SLAVE_COUNT; id++)
    {
        uint16_t _THERMISTOR_VALUES_PER[THERMISTOR_COUNT];
        for (int j = 0; j < THERMISTOR_COUNT; j++)
        {
            _THERMISTOR_VALUES_PER[j] = bms.input.thermistor_resistances[id][j];
        }
        vTaskDelay(pdMS_TO_TICKS(1));
        sendTemp(_THERMISTOR_VALUES_PER, id);
    }
}

void bmsInit()
{
    uint8_t confdat[6 * SLAVE_COUNT];
    memset(confdat, 0, 6 * SLAVE_COUNT);
    for (uint8_t i = 0; i < SLAVE_COUNT; i++)
        confdat[6 * i] = 0b001111100;
    pushCommand(WRCFGA, SLAVE_COUNT, confdat);
}

void openFuseCheck()
{
    int vDrop = initResistance * bms.input.current_adc;
    for (int i = 0; i < SLAVE_COUNT; i++)
    {
        for (int j = 0; j < CELL_COUNT; j++) 
        {
            if ((bms.input.cell_voltages[i][j] - vDrop) > vDropThresh) 
            {
                // do something
            }
        }
    }
}
