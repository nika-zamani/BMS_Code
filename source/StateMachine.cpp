#include "gpio.h"
#include "spi.h"
#include "CanMessage.h"
#include "StateMachine.h"


using namespace BSP;

BMSCollect::BMSCollect(BMSCollectObject o) {
   this->THERMISTOR_VALUES = o.THERMISTOR_VALUES;
   this->SPI_RX = o.SPI_RX;
   this->CELL_VOLTAGES = o.CELL_VOLTAGES;
   this->BMSNumber = o.BMSNumber;
}

StateMachine *StateMachine::instance;

// make structs for each bms (2 pairs in each, 5 total)
StateMachine::StateMachine() {

   uint16_t THERMISTOR_VALUES = 0;  // size tbd
   uint8_t SPI_RX = 0;   // size tbd, received spi value
   uint16_t CELL_VOLTAGES = 0;   // size tbd

   BMSCollectObject c0 = (BMSCollectObject){THERMISTOR_VALUES, SPI_RX, CELL_VOLTAGES, 0};
   BMSCollect *Collection0 = new BMSCollect(c0);

   BMSCollectObject c1 = (BMSCollectObject){THERMISTOR_VALUES, SPI_RX, CELL_VOLTAGES, 1};
   BMSCollect *Collection1 = new BMSCollect(c1);

   BMSCollectObject c2 = (BMSCollectObject){THERMISTOR_VALUES, SPI_RX, CELL_VOLTAGES, 2};
   BMSCollect *Collection2 = new BMSCollect(c2);

   BMSCollectObject c3 = (BMSCollectObject){THERMISTOR_VALUES, SPI_RX, CELL_VOLTAGES, 3};
   BMSCollect *Collection3 = new BMSCollect(c3);

   BMSCollectObject c4 = (BMSCollectObject){THERMISTOR_VALUES, SPI_RX, CELL_VOLTAGES, 4};
   BMSCollect *Collection4 = new BMSCollect(c4);

   BMSCollectObject c5 = (BMSCollectObject){THERMISTOR_VALUES, SPI_RX, CELL_VOLTAGES, 5};
   BMSCollect *Collection5 = new BMSCollect(c5);

   BMSCollectObject c6 = (BMSCollectObject){THERMISTOR_VALUES, SPI_RX, CELL_VOLTAGES, 6};
   BMSCollect *Collection6 = new BMSCollect(c6);

   BMSCollectObject c7 = (BMSCollectObject){THERMISTOR_VALUES, SPI_RX, CELL_VOLTAGES, 7};
   BMSCollect *Collection7 = new BMSCollect(c7);

   BMSCollectObject c8 = (BMSCollectObject){THERMISTOR_VALUES, SPI_RX, CELL_VOLTAGES, 8};
   BMSCollect *Collection8 = new BMSCollect(c8);

   BMSCollectObject c9 = (BMSCollectObject){THERMISTOR_VALUES, SPI_RX, CELL_VOLTAGES, 9};
   BMSCollect *Collection9 = new BMSCollect(c9);

   this->BMSCollection = {Collection0, Collection1, Collection2, Collection3, Collection4, Collection5, Collection6, Collection7, Collection8, Collection9};
}

StateMachine *StateMachine::getInstance() {
   if(instance == 0)
      instance = new StateMachine();
   return instance;
}

BMSCollectArray StateMachine::getBMSCollect() {
   StateMachine *state = getInstance();
   return state->BMSCollection;
}

void StateMachine::setTHERMISTORVALUES(int boardNum, int index, uint16_t data) {
   StateMachine *state = getInstance();
   state->BMSCollection.bmscollect[boardNum]->THERMISTOR_VALUES = data;
}

void StateMachine::setSPIRX(int boardNum, int index, uint8_t *data) {
   StateMachine *state = getInstance();
   state->BMSCollection.bmscollect[boardNum]->SPI_RX = *data;
}

void StateMachine::setCELLVOLTAGES(int boardNum, int index, uint16_t data) {
   StateMachine *state = getInstance();
   state->BMSCollection.bmscollect[boardNum]->CELL_VOLTAGES = data;   // need to figure out layout for voltages
}

void StateMachine::calculateBMSOK() {
   StateMachine *state = getInstance();
}