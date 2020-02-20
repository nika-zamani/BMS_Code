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

/* P0: Evey board sends 32 bit heartbeat(counter) every 10ms that increments after each send 100hz
 */

/* Log current faults and fault history at 10Hz 
 *
 * Faults: 10 Slaves (Max 1040 bits)
 *  Each Slave: (Max 104 bits) 
 *      (4 bits)
 *      DISCONECTED - No response from slave
 *      CVST
 *      AXST
 *      STATST
 * 
 *      Per Cell: (up to 12) (currently 8) (Max 36 bits)
 *          Overvoltage
 *          Undervoltage
 *          Disconected
 * 
 *      Per Thermistor: (up to 16) (currently 14) (Max 64 bits)
 *          Overtemp
 *          Undertemp
 *          Shorted > 2.9v
 *          Disconected < 0.1v
 */

/* voltage and thermistor 16 bit values every 1hz
 */

// Each number is the can bit location
typedef enum{
    //P: Time in Ms
    Time = 0,

    //P+1: Faults
    Apps10InfoFault = 64,
    Apps10NonCriticalFault,
    Apps1OvervoltageInfo,
    Apps1OvervoltageNonCritical,
    Apps2OvervoltageInfo,
    Apps2OvervoltageNonCritical,
    BrakePressure1SoftwareOvervoltageInfo,
    BrakePressure1SoftwareOvervoltageNonCritical,

    //P+3: Apps, Current

    Apps1Data = 64 * 3,
    Apps2Data = (64 * 3) + 16,
    CurrentThrottle1Data = (64 * 3) + 32,
    CurrentThrottle2Data = (64 * 3) + 48,

    //P+4: Brake
    BrakePressure1Data = 64 * 4,
    BrakePressure2Data = (64 * 4) + 16,
    BrakePosition1Data = 64 * 4 + 32,
    BrakePosition2Data = (64 * 4) + 48,

    NULLVAL = 10000


} BitLocation;

union Frame{
    uint64_t faults;
    uint8_t faultsArray[8];
    uint16_t dataArray[4];
};

class CanMessage{
    public:
        static CanMessage* getInstance();        
        void setBitTrue(BitLocation bl);
        void setBitFalse(BitLocation bl);
        void set16(BitLocation bl, uint16_t data);
        void set64(BitLocation bl, uint16_t data);
        void fullSend();
    private:
        union Frame frame[5];
        static CanMessage* instance;
        void initCAN();
        CanMessage();
        void send(uint32_t address, uint8_t *data);
};