#include "../CanMessageStructs/canmessagestructs.h"
#include "CanMessage.h"
#include "taskManager.h"
#include "common.h"
void not_found_id() {}

void canAirs(can::CANlight::frame *f) {
    BmsAirsStruct AirsCanstruct;
    // can::CANlight::frame f = can::CANlight::StaticClass().readrx(0);
    memcpy(&AirsCanstruct, &f, sizeof(BmsAirsStruct));

    // closed is true (1), open is false(0)
    if (AirsCanstruct.airs_positive == 0) {
        gpio::GPIO::StaticClass().clear(GPIO_BMS_AIRS_PORT, GPIO_BMS_AIRS_CH);
    }
    else {
        gpio::GPIO::StaticClass().set(GPIO_BMS_AIRS_PORT, GPIO_BMS_AIRS_CH);
    }
    if (AirsCanstruct.airs_negative == 0) {
        gpio::GPIO::StaticClass().clear(GPIO_BMS_AIRS_PORT, GPIO_BMS_AIRS_CH);
    }
    else {
        gpio::GPIO::StaticClass().set(GPIO_BMS_AIRS_PORT, GPIO_BMS_AIRS_CH);
    }


    // check id (AIRS_ID = correct thing)
    // deal with gpio in here
}

void trans (void *pvParameters) {
    can::CANlight::frame receiveCommand;
    for(;;) {
       xQueueReceiveFromISR(msg_queue, (void *)&receiveCommand, pdFALSE);
        int i = 1;
        switch (receiveCommand.id & 0xFFF) {
            case AIRS_ID: //change according to can message definitions google sheet
                canAirs(&receiveCommand);
                //airs unpack here and check
                break;
            default:
                not_found_id();
        }
    }
}



void transInit() {
    xTaskCreate(trans, "trans", STACK_SIZE, NULL, configMAX_PRIORITIES - 1, NULL);
}