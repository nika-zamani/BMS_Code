#include "../CanMessageStructs/canmessagestructs.h"
#include "CanMessage.h"
#include "taskManager.h"

void not_found_id() {}

void canAirs(can::CANlight::frame *f) {
    BmsAirsStruct AirsCanstruct;
    // can::CANlight::frame f = can::CANlight::StaticClass().readrx(0);
    memcpy(&AirsCanstruct, &f, sizeof(BmsAirsStruct));

    // closed is true (1), open is false(0)
    if (AirsCanstruct.airs_positive == 0) {
        gpio::GPIO::StaticClass().clear(gpio::PortD, 15);
    }
    else {
        gpio::GPIO::StaticClass().set(gpio::PortD, 15);
    }
    if (AirsCanstruct.airs_negative == 0) {
        gpio::GPIO::StaticClass().clear(gpio::PortD, 15);
    }
    else {
        gpio::GPIO::StaticClass().set(gpio::PortD, 15);
    }


    // check id (AIRS_ID = correct thing)
    // deal with gpio in here
}

void trans (void *pvParameters) {
    can::CANlight::frame receiveCommand;
    for(;;) {
        xQueueReceiveFromISR(msg_queue, (void *)&receiveCommand, pdFALSE);
        int i = 1;
        switch (receiveCommand.id) {
            case 0x0336: //change according to can message definitions google sheet
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