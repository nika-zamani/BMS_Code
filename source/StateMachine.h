#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#define NUMBER_BMS_BOARDS 10   // there are 5 pairs of bms boards, but 10 "devices" in general for the LTC chip

typedef struct {
   uint16_t THERMISTOR_VALUES;  // tbd
   uint8_t SPI_RX;   // tbd
   uint16_t CELL_VOLTAGES;   // tbd
   // maybe add current, add whatever the SPI message collects
   int BMSNumber;
}  BMSCollectStruct;

class BMSCollect {
   public:
      BMSCollectStruct o;
      BMSCollect(BMSCollectStruct);
};

typedef struct {
   BMSCollect *bmscollect[NUMBER_BMS_BOARDS];
}  BMSCollectArray;

class StateMachine {
   public:
      StateMachine();
      static StateMachine *instance;
      static StateMachine *getInstance();
      static BMSCollectArray getBMSCollect();
      static void setTHERMISTORVALUES(int boardNum, uint16_t data);
      static void setSPIRX(int boardNum, uint8_t *data);
      static void setCELLVOLTAGES(int boardNum, uint16_t data);
      static void calculateBMSOK();
      static uint8_t getSPIRX(int boardNum);
      static uint16_t getTHERMISTORVALUES(int boardNum);
      static uint16_t getCELLVOLTAGES(int boardNum);
      BMSCollectArray BMSCollection;
};


#endif