#ifndef DEFINES_H_
#define DEFINES_H_

#define slaves 2        // Total slaves
#define commlen 4+8*slaves // length of most isospi transactions
#define cells 11        // Cells per slave
#define thermistors 16  // Thermistors per slave

#define voltageLimitUpper 42000
#define voltageLimitLower 28000

#define tempLimitUpper 50000
#define tempLimitLower 0
#define tempconfig 0b1111111111000011 // 1=cell, 0=aux

// System ticks: handler called at .1ms
// Smallest quantum is 100us
#define CLOCKHZ 60000000U
#define SYSTICK 6000U
#define us(x) x/100
#define ms(x) x*10

#define ltccsport BSP::gpio::PortA
#define ltccspin 2

#endif
