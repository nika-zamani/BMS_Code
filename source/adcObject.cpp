
// For tick count imports
#include "FreeRTOS.h"
#include "task.h"
#include "pin_mux.h"
#include "MKE18F16.h"  
#include "adc.h"

#include "adcObject.h"


using namespace BSP;


adcObject::adcObject(ADC_Type *adc, int channel) {
   this->adc = adc;
   this->channel = channel;
}

void adcObject::adcRead() {
   adc::ADC &adcTemp = adc::ADC::StaticClass();
   this->adc_data = adcTemp.read(adc, channel);
}

void adcObject::initADC() {
   adc::ADC::ConstructStatic(NULL);
}
