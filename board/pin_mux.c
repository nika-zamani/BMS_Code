/***********************************************************************************************************************
 * This file was generated by the MCUXpresso Config Tools. Any manual edits made to this file
 * will be overwritten if the respective MCUXpresso Config Tools is used to update this file.
 **********************************************************************************************************************/

/* clang-format off */
/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
!!GlobalInfo
product: Pins v5.0
processor: MKE18F512xxx16
package_id: MKE18F512VLH16
mcu_data: ksdk2_0
processor_version: 5.0.0
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */
/* clang-format on */

#include "fsl_common.h"
#include "fsl_port.h"
#include "pin_mux.h"

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : BOARD_InitBootPins
 * Description   : Calls initialization functions.
 *
 * END ****************************************************************************************************************/
void BOARD_InitBootPins(void)
{
    BOARD_InitPins();
}

/* clang-format off */
/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
BOARD_InitPins:
- options: {callFromInitBoot: 'true', coreID: core0, enableClock: 'true'}
- pin_list:
  - {pin_num: '60', peripheral: LPSPI0, signal: SCK, pin_signal: ADC2_SE7/PTE0/LPSPI0_SCK/TCLK1/LPI2C1_SDA/FTM1_FLT2}
  - {pin_num: '59', peripheral: LPSPI0, signal: SIN, pin_signal: ADC2_SE6/PTE1/LPSPI0_SIN/LPI2C0_HREQ/LPI2C1_SCL/FTM1_FLT1}
  - {pin_num: '54', peripheral: LPSPI0, signal: SOUT, pin_signal: ADC1_SE10/PTE2/LPSPI0_SOUT/LPTMR0_ALT3/FTM3_CH6/PWT_IN3/LPUART1_CTS}
  - {pin_num: '53', peripheral: GPIOE, signal: 'GPIO, 6', pin_signal: ADC1_SE11/ACMP0_IN6/PTE6/LPSPI0_PCS2/FTM3_CH7/LPUART1_RTS}
  - {pin_num: '46', peripheral: GPIOD, signal: 'GPIO, 2', pin_signal: ADC1_SE2/PTD2/FTM3_CH4/LPSPI1_SOUT/FXIO_D4/TRGMUX_IN5}
  - {pin_num: '47', peripheral: GPIOA, signal: 'GPIO, 3', pin_signal: ADC1_SE1/PTA3/FTM3_CH1/LPI2C0_SCL/EWM_IN/LPUART0_TX}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */
/* clang-format on */

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : BOARD_InitPins
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 * END ****************************************************************************************************************/
void BOARD_InitPins(void)
{
    /* Clock Control: Clock enabled */
    CLOCK_EnableClock(kCLOCK_PortA);
    /* Clock Control: Clock enabled */
    CLOCK_EnableClock(kCLOCK_PortD);
    /* Clock Control: Clock enabled */
    CLOCK_EnableClock(kCLOCK_PortE);

    /* PORTA3 (pin 47) is configured as PTA3 */
    PORT_SetPinMux(PORTA, 3U, kPORT_MuxAsGpio);

    /* PORTD2 (pin 46) is configured as PTD2 */
    PORT_SetPinMux(PORTD, 2U, kPORT_MuxAsGpio);

    /* PORTE0 (pin 60) is configured as LPSPI0_SCK */
    PORT_SetPinMux(PORTE, 0U, kPORT_MuxAlt2);

    /* PORTE1 (pin 59) is configured as LPSPI0_SIN */
    PORT_SetPinMux(PORTE, 1U, kPORT_MuxAlt2);

    /* PORTE2 (pin 54) is configured as LPSPI0_SOUT */
    PORT_SetPinMux(PORTE, 2U, kPORT_MuxAlt2);

    /* PORTE6 (pin 53) is configured as PTE6 */
    PORT_SetPinMux(PORTE, 6U, kPORT_MuxAsGpio);
}
/***********************************************************************************************************************
 * EOF
 **********************************************************************************************************************/
