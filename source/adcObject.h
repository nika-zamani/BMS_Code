#ifndef ADCOBJECT_H
#define ADCOBJECT_H

#include "adc.h"
#include "ADC.h"
#include "MKE18F16.h"



class adcObject {

private:
   ADC_Type *adc;
   int channel;

public:
   adcObject(ADC_Type *adc, int channel);
   void adcRead();
   static void initADC();
   uint16_t adc_data;
};

#endif