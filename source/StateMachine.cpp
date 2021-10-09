#include "gpio.h"
#include "spi.h"
#include "CanMessage.h"
#include "StateMachine.h"


using namespace BSP;

BMSCollect::BMSCollect(BMSCollectStruct o) {
   this->o = o;
}

StateMachine *StateMachine::instance;

StateMachine::StateMachine() {

   // uint16_t THERMISTOR_VALUES = 0;  // size tbd
   // uint8_t SPI_RX = 0;   // size tbd, received spi value
   // uint16_t CELL_VOLTAGES = 0;   // size tbd

   BMSCollectStruct c0 = (BMSCollectStruct){0, 0, 0, 0};
   BMSCollect *Collection0 = new BMSCollect(c0);

   BMSCollectStruct c1 = (BMSCollectStruct){0, 0, 0, 1};
   BMSCollect *Collection1 = new BMSCollect(c1);

   BMSCollectStruct c2 = (BMSCollectStruct){0, 0, 0, 2};
   BMSCollect *Collection2 = new BMSCollect(c2);

   BMSCollectStruct c3 = (BMSCollectStruct){0, 0, 0, 3};
   BMSCollect *Collection3 = new BMSCollect(c3);

   BMSCollectStruct c4 = (BMSCollectStruct){0, 0, 0, 4};
   BMSCollect *Collection4 = new BMSCollect(c4);

   BMSCollectStruct c5 = (BMSCollectStruct){0, 0, 0, 5};
   BMSCollect *Collection5 = new BMSCollect(c5);

   BMSCollectStruct c6 = (BMSCollectStruct){0, 0, 0, 6};
   BMSCollect *Collection6 = new BMSCollect(c6);

   BMSCollectStruct c7 = (BMSCollectStruct){0, 0, 0, 7};
   BMSCollect *Collection7 = new BMSCollect(c7);

   BMSCollectStruct c8 = (BMSCollectStruct){0, 0, 0, 8};
   BMSCollect *Collection8 = new BMSCollect(c8);

   BMSCollectStruct c9 = (BMSCollectStruct){0, 0, 0, 9};
   BMSCollect *Collection9 = new BMSCollect(c9);

   this->BMSCollection = {Collection0, Collection1, Collection2, Collection3, Collection4, Collection5, Collection6, Collection7, Collection8, Collection9};
}

StateMachine *StateMachine::getInstance() {
   if(instance == 0)
      instance = new StateMachine();
   return instance;
}

// BMSCollectArray StateMachine::getBMSCollect() {
//    StateMachine *state = getInstance();
//    return state->BMSCollection;
// }

// void StateMachine::setTHERMISTORVALUES(int boardNum, uint16_t data) {
//    StateMachine *state = getInstance();
//    state->BMSCollection.bmscollect[boardNum]->o.THERMISTOR_VALUES = data;
//    // memcpy(&(state->BMSCollection.bmscollect[boardNum]->o.THERMISTOR_VALUES), &data, 6);
// }

void StateMachine::setSPIRX(int boardNum, uint8_t *data) {
   StateMachine *state = getInstance();
   // memcpy(&(state->BMSCollection.bmscollect[boardNum]->o.SPI_RX), data, 6);
   // state->BMSCollection.bmscollect[boardNum]->o.SPI_RX = *data;
}

// void StateMachine::setCELLVOLTAGES(int boardNum, uint16_t data) {
//    StateMachine *state = getInstance();
//    state->BMSCollection.bmscollect[boardNum]->o.CELL_VOLTAGES = data;   // need to figure out layout for voltages
//    // memcpy(&(state->BMSCollection.bmscollect[boardNum]->o.THERMISTOR_VALUES), &data, 6);
// }

// void StateMachine::calculateBMSOK() {
//    StateMachine *state = getInstance();
// }

// uint8_t StateMachine::getSPIRX(int boardNum) {
//    StateMachine *state = getInstance();
//    return state->BMSCollection.bmscollect[boardNum]->o.SPI_RX;
// }

// uint16_t StateMachine::getTHERMISTORVALUES(int boardNum) {
//    StateMachine *state = getInstance();
//    return state->BMSCollection.bmscollect[boardNum]->o.THERMISTOR_VALUES;
// }

// uint16_t StateMachine::getCELLVOLTAGES(int boardNum) {
//    StateMachine *state = getInstance();
//    return state->BMSCollection.bmscollect[boardNum]->o.CELL_VOLTAGES;
// }