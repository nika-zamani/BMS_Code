#ifndef CAN_MESSAGE
#define CAN_MESSAGE

#include <unordered_map>

#define PEDDLE_BOX_ID 0x400 + 0x1

#define CAN_BUS 0
#define CAN_BAUD_RATE 250000

#define MC_CAN_BUS 1
#define MC_CAN_BAUD_RATE 1000000

#define RELUCTOR_TEETH 20
#define TIMER_FREQUENCY 125000

#define PWM_FREQUENCY 25000
#define PWM_MIN 5
#define PWM_MAX 95

/*
 * TOTAL CAN ADRESS SPACE REQUIRED: 856.25 Bytes
 */

/* P0: Evey board sends 32 bit heartbeat(counter) every 10ms that increments after each send 100hz
 */

/* Log current faults and fault history at 10Hz 
 *
 * Faults: (1282 bits) (160.25 Bytes)
 * 
 *  Slaves: (up to 12) (Max 48 bits)
 *      (4 bits)
 *      DISCONECTED - No response from slave
 *      CVST
 *      AXST
 *      STATST
 * 
 *  Cell Faults: (up to 142) (Max 426 bits)
 *          Overvoltage
 *          Undervoltage
 *          Disconected
 * 
 *  Thermistor Faults: (up to 202) (Max 808 bits)
 *          Disconected < 0.1v
 *          Shorted > 2.9v
 *          Undertemp
 *          Overtemp
 * 
 */

/* voltage and thermistor 16 bit values every 1hz
 *
 *  Cell Values: (Max 2272 Bits)
 * 
 *  Thermistor Values: (Max 3232 Bits)
 *
 * Thermistors at Cells + 36
 * Naxt at Thermistors + 51
 */

typedef enum {
    Heartbeat = 0,

    Slaves = 64,
    CellFaults = 112,
    ThermistorFaults = 538,
    CellValues = 1346, 
    ThermistorValues = 3618, 
    SectionEnd = 6850
} sectionBitOffsets;

// Each number is the can bit location
typedef enum{
    SlaveFaultOffsetsDisconected = 0,
    FailedCVST,
    FailedAXST,
    FailedSTATST
} SlaveFaultOffsets;

typedef enum{
    Undervoltage = 0,
    Overvoltage,
    CellFaultOffsetsDisconected
} CellFaultOffsets;

typedef enum{
    ThermistorFaultOffsetsDisconected = 0,
    Shorted,
    Undertemp,
    Overtemp
} ThermistorFaultOffsets;

uint64_t heartbeat;
uint8_t faultsCurrent[161];
uint8_t faultsHistory[161];
uint16_t dataArray[688];

union Frame{
    uint64_t faults;
    uint8_t faultsArray[8];
    uint16_t dataArray[4];
};

class CanMessage{
    public:
        static CanMessage* getInstance();        
        // void setBitTrue(BitLocation bl);
        // void setBitFalse(BitLocation bl);
        // void set16(BitLocation bl, uint16_t data);
        // void set64(BitLocation bl, uint16_t data);
        void fullSend();
        int getSlaveOffset(int slaveNumber) { return Slaves + (slaveNumber * 4); }
        int getCellFaultOffset(int cellNumber) { return CellFaults + (cellNumber * 3); }
        int getThermistorFaultOffset(int thermistorNumber) { return ThermistorFaults + (thermistorNumber * 4); }
        int getCellValueOffset(int cellNumber) { return CellValues + (cellNumber * 16);}
        int getThermistorValueOffset(int thermistorNumber) { return ThermistorValues + (thermistorNumber * 16); }
    private:
        union Frame frame[5];
        static CanMessage* instance;
        void initCAN();
        CanMessage();
        void send(uint32_t address, uint8_t *data);
};

#endif