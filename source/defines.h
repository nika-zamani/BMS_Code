#ifndef DEFINES_H_
#define DEFINES_H_

#define slaves 1        // Total slaves
#define cells 11        // Cells per slave
#define thermistors 12  // Thermistors per slave

#define voltageLimitUpper 42000
#define voltageLimitLower 28000

#define tempLimitUpper 0
#define tempLimitLower 0

// System ticks: handler called at .1ms
// Smallest quantum is 100us
#define CLOCKHZ 60000000U
#define SYSTICK 6000U
#define us(x) 100/x
#define ms(x) 10*x

#define ltccsport gpio::PortA
#define ltccspin 2

#endif
