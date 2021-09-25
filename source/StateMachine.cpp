#include "gpio.h"
#include "spi.h"
#include "CanMessage.h"
#include "StateMachine.h"


using namespace BSP;

BMSCollect::BMSCollect(BMSCollectStruct o) {
   this->o = o;
}

StateMachine *StateMachine::instance;

// make structs for each bms (2 pairs in each, 5 total)
StateMachine::StateMachine() {

   uint16_t THERMISTOR_VALUES = 0;  // size tbd
   uint8_t SPI_RX = 0;   // size tbd, received spi value
   uint16_t CELL_VOLTAGES = 0;   // size tbd

   BMSCollectStruct c0 = (BMSCollectStruct){THERMISTOR_VALUES, SPI_RX, CELL_VOLTAGES, 0};
   BMSCollect *Collection0 = new BMSCollect(c0);

   BMSCollectStruct c1 = (BMSCollectStruct){THERMISTOR_VALUES, SPI_RX, CELL_VOLTAGES, 1};
   BMSCollect *Collection1 = new BMSCollect(c1);

   BMSCollectStruct c2 = (BMSCollectStruct){THERMISTOR_VALUES, SPI_RX, CELL_VOLTAGES, 2};
   BMSCollect *Collection2 = new BMSCollect(c2);

   BMSCollectStruct c3 = (BMSCollectStruct){THERMISTOR_VALUES, SPI_RX, CELL_VOLTAGES, 3};
   BMSCollect *Collection3 = new BMSCollect(c3);

   BMSCollectStruct c4 = (BMSCollectStruct){THERMISTOR_VALUES, SPI_RX, CELL_VOLTAGES, 4};
   BMSCollect *Collection4 = new BMSCollect(c4);

   BMSCollectStruct c5 = (BMSCollectStruct){THERMISTOR_VALUES, SPI_RX, CELL_VOLTAGES, 5};
   BMSCollect *Collection5 = new BMSCollect(c5);

   BMSCollectStruct c6 = (BMSCollectStruct){THERMISTOR_VALUES, SPI_RX, CELL_VOLTAGES, 6};
   BMSCollect *Collection6 = new BMSCollect(c6);

   BMSCollectStruct c7 = (BMSCollectStruct){THERMISTOR_VALUES, SPI_RX, CELL_VOLTAGES, 7};
   BMSCollect *Collection7 = new BMSCollect(c7);

   BMSCollectStruct c8 = (BMSCollectStruct){THERMISTOR_VALUES, SPI_RX, CELL_VOLTAGES, 8};
   BMSCollect *Collection8 = new BMSCollect(c8);

   BMSCollectStruct c9 = (BMSCollectStruct){THERMISTOR_VALUES, SPI_RX, CELL_VOLTAGES, 9};
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
   state->BMSCollection.bmscollect[boardNum]->o.THERMISTOR_VALUES = data;
}

void StateMachine::setSPIRX(int boardNum, int index, uint8_t *data) {
   StateMachine *state = getInstance();
   state->BMSCollection.bmscollect[boardNum]->o.SPI_RX = *data;
}

void StateMachine::setCELLVOLTAGES(int boardNum, int index, uint16_t data) {
   StateMachine *state = getInstance();
   state->BMSCollection.bmscollect[boardNum]->o.CELL_VOLTAGES = data;   // need to figure out layout for voltages
}

void StateMachine::calculateBMSOK() {
   StateMachine *state = getInstance();
}