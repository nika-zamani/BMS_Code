#ifndef DEFINES_H_
#define DEFINES_H_

#define slaves 8        // Total slaves
#define confThresh 6
#define commlen 4+8*slaves // length of most isospi transactions
#define cells 11        // Cells per slave
#define thermistors 16  // Thermistors per slave

#define voltageLimitUpper 42000
#define voltageLimitLower 28000

#define tempLimitUpper 20080 // 10C
#define tempLimitLower 6000 // 60C
#define tempBrokenUpper 28000 // temp shorted to rail
#define tempBrokenLower 2000 // temp shorted to ground
#define tempconfig 0b1100001111111111 // 1=cell, 0=aux
#define tempTotal 12*slaves // total measured cell temps
#define tempExtra 6 // allowable broken temps

// System ticks: handler called at .1ms
// Smallest quantum is 100us
#define CLOCKHZ 60000000U
#define SYSTICK 6000U
#define us(x) x/100
#define ms(x) x*10

#define ltccsport BSP::gpio::PortA
#define ltccspin 2

#define bmsokport BSP::gpio::PortD
#define bmsokpin 0
#endif
