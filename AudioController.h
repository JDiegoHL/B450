// AudioController.h
#pragma once
#include "SystemState.h"

class AudioController {
private:
    SystemState& state;
    HardwareSerial* nanoSerial;

public:
    AudioController(SystemState& systemState) 
        : state(systemState) {
        nanoSerial = new HardwareSerial(2);
    }
    
    void begin() {
        nanoSerial->begin(9600, SERIAL_8N1, 16, 17);
    }
    
    void setMode(uint8_t mode) {
        state.audioMode = mode;
        // Implementar comunicación con Arduino Nano
    }
};
