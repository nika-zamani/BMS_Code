// look at pedal box 2020

// Build 8 byte can messages from fault bits
// 29 bits of adress space
#include <iostream>
#include <cmath>
#include "clock_config.h"
#include "canlight.h"
#include "Scheduler.h"
#include "pin_mux.h"
#include "MKE18F16.h"
#include "CanMessage.h"

using namespace BSP;

CanMessage* CanMessage::instance;

CanMessage::CanMessage(){
    
}

CanMessage* CanMessage::getInstance(){
    if(instance == 0){
        instance = new CanMessage();
        instance->initCAN();
        
    }
    return instance;
}

void CanMessage::setBitTrue(BitLocation bl){
    int frameNum = floor(int(bl) / 64.0);
    int byteNum = floor((int(bl) % 64)/8);
    int bitNum = bl % 8;
    frame[frameNum].faultsArray[byteNum] = 
     frame[frameNum].faultsArray[byteNum] | ( 0x01 << bitNum);
}

void CanMessage::setBitFalse(BitLocation bl){
    int frameNum = (int) floor(bl / 64.0);
    int byteNum = (int) floor((bl % 64)/8);
    int bitNum = bl % 8;
    frame[frameNum].faultsArray[byteNum] = 
     frame[frameNum].faultsArray[byteNum] & (~ ( 0x01 << bitNum));
}

void CanMessage::set16(BitLocation bl, uint16_t data){
    int frameNum = (int) floor(bl / 64.0);
    int byteNum = (int) floor((bl % 64)/16);
    frame[frameNum].dataArray[byteNum] = data;
}

void CanMessage::set64(BitLocation bl, uint16_t data){
    int frameNum = (int) floor(bl / 64.0);
    frame[frameNum].faults = data;
}

void CanMessage::initCAN() {
    // Zero Can Message
    memset(&(this->frame), 0, sizeof(this->frame));

    // Setup CAN Config
    can::canlight_config config;
    can::CANlight::canx_config canx_config;
   
    // Initialize CAN driver
    can::CANlight::ConstructStatic(&config);
    can::CANlight &can = can::CANlight::StaticClass();
    
    // Configure CAN bus
    canx_config.baudRate = CAN_BAUD_RATE;
    can.init(CAN_BUS, &canx_config);
}

// sends a CAN message on the specified bus
void CanMessage::send(uint32_t address, uint8_t *data) {
    can::CANlight &can = can::CANlight::StaticClass();
    can::CANlight::frame frame;

    frame.id = address;
    frame.ext = 1;
    frame.dlc = 8;
    memcpy(frame.data, data, sizeof(frame.data));

    // Only one bus
    can.tx(CAN_BUS, frame);
}

//Send all CAN messages
void CanMessage::fullSend(){
    send(PEDDLE_BOX_ID, CanMessage::frame[0].faultsArray);
    send(PEDDLE_BOX_ID+1, CanMessage::frame[1].faultsArray);
    send(PEDDLE_BOX_ID+2, CanMessage::frame[2].faultsArray);
    send(PEDDLE_BOX_ID+3, CanMessage::frame[3].faultsArray);
    send(PEDDLE_BOX_ID+4, CanMessage::frame[4].faultsArray);
}
