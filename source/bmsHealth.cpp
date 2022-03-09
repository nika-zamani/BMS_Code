#include "bmsHealth.h"

uint8_t RETURN_DATA[6 * SLAVE_COUNT];
uint8_t _THERMISTOR_INDEXES[8] = {0, 3, 4, 6, 7, 9, 10, 13};
uint8_t SCONTROL_DATA[6 * SLAVE_COUNT];
const uint8_t _DCP = 0; // discharge not permited

extern BMS bms;

uint32_t calcVoltFromTemp(uint16_t temperature)
{
    // 9*(10^-5)*(x^4)-3*(10^-2)*(x^3)+9*(10^-1)*(x^2)+262*x+8266
    uint32_t v1 = (.00009 * temperature * temperature * temperature * temperature) - (.03 * temperature * temperature * temperature) + (.9 * temperature * temperature) + (262 * temperature) + 8266;
    return v1;
}

uint16_t calcTempFromVolt(uint16_t voltage)
{
    // y = -0.0002x^2 + 0.0358x + 0.7128
    float vFloat = voltage * 0.0001;
    float t1 = ((-0.0002 * vFloat * vFloat) + (0.0358 * vFloat) + (0.7128)) * 100;

    return t1 * 100;
}

uint16_t getMaxTemp()
{
    uint16_t tempMax = 0;
    for (int i = 0; i < SLAVE_COUNT; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            if (tempMax < bms.input.thermistor_values[i][_THERMISTOR_INDEXES[j]])
            {
                tempMax = bms.input.thermistor_values[i][_THERMISTOR_INDEXES[j]];
            }
        }
    }
    return tempMax;
}

uint32_t getSumVoltage()
{
    uint32_t sumVoltage = 0;
    for (int i = 0; i < SLAVE_COUNT; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            sumVoltage += bms.input.cell_voltages[i][j];
        }
    }
    return sumVoltage;
}

void getVoltages(uint8_t md)
{
    int error = 0;
    memset(RETURN_DATA, 0, sizeof(RETURN_DATA));
    error = pushCommand(CLRSCTRL, SLAVE_COUNT, RETURN_DATA);

    pushCommand(ADCVSC, SLAVE_COUNT, RETURN_DATA, md, _DCP);

    error = pushCommand(RDCVA, SLAVE_COUNT, RETURN_DATA);
    for (int i = 0; i < SLAVE_COUNT; i++)
    {
        memcpy((void *)&bms.input.cell_voltages[i], (RETURN_DATA + (i * 6)), 6);
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

    error = pushCommand(RDSTATA, SLAVE_COUNT, RETURN_DATA);
}

void SControl()
{

    int error = 0;
    memset(SCONTROL_DATA, 0, sizeof(SCONTROL_DATA));
    memset(RETURN_DATA, 0, sizeof(RETURN_DATA));
    error = pushCommand(CLRSCTRL, SLAVE_COUNT, RETURN_DATA);
    error = pushCommand(ADCVSC, SLAVE_COUNT, RETURN_DATA, 1, _DCP); // is this needed?
    SCONTROL_DATA[0] = 0b10000000;
    SCONTROL_DATA[1] = 0b10001000;
    SCONTROL_DATA[2] = 0b10001000;
    SCONTROL_DATA[3] = 0b10000000;
    SCONTROL_DATA[4] = 0b10001000;
    SCONTROL_DATA[5] = 0b10001000;
    pushCommand(STSCTRL, SLAVE_COUNT, RETURN_DATA); // put this here?????
    for (int i = 0; i < 6; i++)
    {
        uint8_t temp = SCONTROL_DATA[5];
        SCONTROL_DATA[5] = SCONTROL_DATA[4];
        SCONTROL_DATA[4] = SCONTROL_DATA[3];
        SCONTROL_DATA[3] = SCONTROL_DATA[2];
        SCONTROL_DATA[2] = SCONTROL_DATA[1];
        SCONTROL_DATA[1] = SCONTROL_DATA[0];
        SCONTROL_DATA[0] = temp;
        memset(RETURN_DATA, 0, sizeof(RETURN_DATA));
        error = pushCommand(WRSCTRL, SLAVE_COUNT, SCONTROL_DATA);
        vTaskDelay(100); // check ticks
        error = pushCommand(RDSCTRL, SLAVE_COUNT, RETURN_DATA);
    }
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
    uint8_t CHG = 0b000;
    memset(RETURN_DATA, 0, sizeof(RETURN_DATA));

    for (int j = 0; j < 8; j++)
    {

        muxSet(0, j);
        muxSet(1, j);

        error = pushCommand(ADCVAX, SLAVE_COUNT, RETURN_DATA, md);
        vTaskDelay(50);
        error = pushCommand(RDAUXA, SLAVE_COUNT, RETURN_DATA);

        for (int i = 0; i < SLAVE_COUNT; i++)
        {
            bms.input.thermistor_values[i][j] = RETURN_DATA[i * 6] | RETURN_DATA[i * 6 + 1] << 8; // add 4 when it is collected; store raw data for now
            bms.input.thermistor_values[i][j + 8] = RETURN_DATA[i * 6 + 2] | (RETURN_DATA[i * 6 + 3] << 8);
        }
    }
}

void calculateBMS_OK(uint32_t voltTempLimit)
{
    for (int i = 0; i < SLAVE_COUNT; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            if ((bms.input.cell_voltages[i][j] < 28000) | (bms.input.cell_voltages[i][j] > 45000))
            {
                bms.output.bms_ok = false;
                return;
            }
            if (bms.input.thermistor_values[i][_THERMISTOR_INDEXES[j]] > voltTempLimit)
            {
                bms.output.bms_ok = false;
                return;
            }
        }
    }
    bms.output.bms_ok = true;
}

void measureSendVoltageTempCurrent()
{
    bms.input.sum_voltage = getSumVoltage();
    bms.input.max_temp = calcTempFromVolt(getMaxTemp());
    bms.input.current_adc = measureCurrent();
    sendMainVoltageTempCurrent(bms.input.sum_voltage, bms.input.max_temp, bms.input.current_adc);
}

void sendVoltages()
{
    for (int id = 0; id < SLAVE_COUNT; id++)
    {
        sendVoltage((uint16_t *)&bms.input.cell_voltages[id], id);
    }
}

void sendTemperatures()
{
    for (int id = 0; id < SLAVE_COUNT; id++)
    {
        uint16_t _THERMISTOR_VALUES_PER[8];
        for (int j = 0; j < 8; j++)
        {
            _THERMISTOR_VALUES_PER[j] = bms.input.thermistor_values[id][_THERMISTOR_INDEXES[j]];
        }
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

void monitorBMSHealth(void *pvParameters)
{
    TickType_t xLastWakeTime;
    const TickType_t period = tick_ms(1000);

    for (;;)
    {
        xLastWakeTime = xTaskGetTickCount();

        // SControl();

        vTaskDelayUntil(&xLastWakeTime, period);
    }
}
