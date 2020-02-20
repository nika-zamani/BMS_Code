#include "MKE18F16.h"
#include "pin_mux.h"
#include "clock_config.h"

#include "gpio.h"

using namespace BSP;

int main(void) {
    BOARD_InitBootPins();
    BOARD_InitBootClocks();

    gpio::GPIO::clear(gpio::PortE, 3); // MCU_Relay

    gpio::GPIO::set(gpio::PortE, 3); // MCU_Relay

    // SysTick_Config(60000);

    for(;;);

    return 0;
}

extern "C" {
    void SysTick_Handler(void){

        static uint64_t time = 0;
        time++;

        static volatile uint8_t WheelSpeed1;;
        WheelSpeed1 = gpio::GPIO::read(gpio::PortC, 2);


        if(time > 1000){

            gpio::GPIO::toggle(gpio::PortE, 3); // MCU_Relay

            time = 0;
        }
    }
}
