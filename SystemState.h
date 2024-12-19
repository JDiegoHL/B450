// SystemState.h
#pragma once

struct SystemState {
    uint8_t currentPhase;
    struct {
        uint8_t r;
        uint8_t g;
        uint8_t b;
    } rgb[2];
    uint8_t auxiliary[5];
    bool relay[2];
    uint8_t audioMode;
    uint8_t currentTrack;
    
    SystemState() : currentPhase(0), audioMode(0), currentTrack(1) {
        for(int i = 0; i < 2; i++) {
            rgb[i] = {0, 0, 0};
        }
        for(int i = 0; i < 5; i++) {
            auxiliary[i] = 0;
        }
        relay[0] = relay[1] = false;
    }
};
