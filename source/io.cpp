#include "io.h"

extern BMS bms;

extern void bmsspicb();

void prvSetupHardware(void)
{

    BOARD_InitBootClocks();
    BOARD_InitBootPins();

    gpio::GPIO::ConstructStatic();
    gpio::GPIO &gpio = gpio::GPIO::StaticClass();
    // Set out direction GPIO
    gpio.out_dir(GPIO_BMS_OK_PORT, GPIO_BMS_OK_CH);
    gpio.out_dir(GPIO_AIR_POS_PORT, GPIO_AIR_POS_CH);
    gpio.out_dir(GPIO_AIR_NEG_PORT, GPIO_AIR_NEG_CH);
    gpio.out_dir(GPIO_PRECHARGE_PORT, GPIO_PRECHARGE_CH);
    gpio.out_dir(GPIO_DCDC_EN_PORT, GPIO_DCDC_EN_CH);

    canInit();

    adc::ADC::ConstructStatic(NULL);

    spi::spi_config conf;
    conf.callbacks[0] = bmsspicb;
    spi::SPI::ConstructStatic(&conf);
    spi::SPI &spi = spi::SPI::StaticClass();

    spi::SPI::masterConfig mconf;
    mconf.baudRate = ltcbaud / 10;
    mconf.csport = ltccsport;
    mconf.cspin = ltccspin;
    spi.initMaster(0, &mconf);

    NVIC->IP[26] |= 6 << 4;
}

uint16_t measureCurrent()
{
    adc::ADC &adc = adc::ADC::StaticClass();
    uint16_t currentADC = adc.read(ADC_CURRENT_BASE, ADC_CURRENT_CH); // 10-11mV per 5A
    //currentADC = ((currentADC * 5) / 0.0105) * 100;                   // (current) * (5A/10.5mV) * (1mV/.001V)
    return currentADC;
}

void unpackCanAirPrechargeDcdcEnable(can::CANlight::frame *f)
{
    BmsAirsPreChargeDCDCEnableStruct data;
    gpio::GPIO &gpio = gpio::GPIO::StaticClass();

    memcpy(&data, f->data, sizeof(BmsAirsPreChargeDCDCEnableStruct));

    bms.output.airs_positive = data.airs_positive;
    bms.output.airs_negative = data.airs_negative;
    bms.output.precharge = data.precharge;
    bms.output.dcdc_enable = data.dcdc_enable;

    data.airs_positive && bms.input.ts_ready ? gpio.set(GPIO_AIR_POS_PORT, GPIO_AIR_POS_CH) : gpio.clear(GPIO_AIR_POS_PORT, GPIO_AIR_POS_CH);
    data.airs_negative && bms.input.ts_ready ? gpio.set(GPIO_AIR_NEG_PORT, GPIO_AIR_NEG_CH) : gpio.clear(GPIO_AIR_NEG_PORT, GPIO_AIR_NEG_CH);
    data.precharge && bms.input.ts_ready ? gpio.set(GPIO_PRECHARGE_PORT, GPIO_PRECHARGE_CH) : gpio.clear(GPIO_PRECHARGE_PORT, GPIO_PRECHARGE_CH);
    data.dcdc_enable && bms.input.ts_ready && !bms.input.dcdc_fault ? gpio.set(GPIO_DCDC_EN_PORT, GPIO_DCDC_EN_CH) : gpio.clear(GPIO_DCDC_EN_PORT, GPIO_DCDC_EN_CH);
}

void setBMS_OK(){
    gpio::GPIO &gpio = gpio::GPIO::StaticClass();

    bms.output.bms_ok ? gpio.set(GPIO_BMS_OK_PORT, GPIO_BMS_OK_CH) : gpio.clear(GPIO_BMS_OK_PORT, GPIO_BMS_OK_CH);
}

void readDcdcTemp()
{
    adc::ADC &adc = adc::ADC::StaticClass();
    uint16_t tempADC = adc.read(ADC_DCDC_TEMP_BASE, ADC_DCDC_TEMP_CH);
    bms.input.dcdc_temp = ((tempADC / 4095.0 * 5.0)) * 50.76142 * 1000;
}

void readGpioInputs()
{
    gpio::GPIO &gpio = gpio::GPIO::StaticClass();

    bms.input.ts_live = gpio.read(GPIO_TS_LIVE_PORT, GPIO_TS_LIVE_CH);
    bms.input.ts_ready = gpio.read(GPIO_TS_READY_PORT, GPIO_TS_READY_CH);
    bms.input.dcdc_fault = gpio.read(GPIO_DCDC_FAULT_PORT, GPIO_DCDC_FAULT_CH);

    if (!bms.input.ts_ready){
        gpio.clear(GPIO_AIR_POS_PORT, GPIO_AIR_POS_CH);
        gpio.clear(GPIO_AIR_NEG_PORT, GPIO_AIR_NEG_CH);
        gpio.clear(GPIO_PRECHARGE_PORT, GPIO_PRECHARGE_CH);
        gpio.clear(GPIO_DCDC_EN_PORT, GPIO_DCDC_EN_CH);
    }
}

void not_found_id()
{
}

void taskDequeueCan(void *pvParameters)
{
    can::CANlight::frame receiveCommand;
    for (;;)
    {
        xQueueReceive(msg_queue, (void *)&receiveCommand, portMAX_DELAY);
        switch (receiveCommand.id & 0xFFF)
        {
        case AIRS_ID:
            unpackCanAirPrechargeDcdcEnable(&receiveCommand);
            break;
        case CHARGING_ID:
            bms.input.is_charging = true;
        default:
            not_found_id();
        }
    }
}