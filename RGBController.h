// RGBController.h
#pragma once
#include <Adafruit_PWMServoDriver.h>
#include "SystemState.h"

class RGBController {
private:
    Adafruit_PWMServoDriver& pwm;
    SystemState& state;

    // Constantes para los canales PWM
    static const int RGB1_BASE_CHANNEL = 0;  // Canales 0, 1, 2 para RGB1
    static const int RGB2_BASE_CHANNEL = 3;  // Canales 3, 4, 5 para RGB2
    static const int AUX_BASE_CHANNEL = 6;   // Canales 6-10 para auxiliares
    
    // Función auxiliar para convertir valores de 8 bits a 12 bits
    uint16_t convert8to12Bits(uint8_t value) {
        return map(value, 0, 255, 0, 4095);
    }

public:
    RGBController(Adafruit_PWMServoDriver& pwmDriver, SystemState& systemState)
        : pwm(pwmDriver), state(systemState) {}

    void begin() {
        pwm.begin();
        pwm.setPWMFreq(1000);  // Frecuencia PWM para un control suave
        
        // Inicialmente apagamos todos los canales
        for (int i = 0; i < 11; i++) {  // 6 canales RGB + 5 auxiliares
            pwm.setPWM(i, 0, 0);
        }
    }

    // Establece un color RGB completo para un canal específico
    void setRGBColor(uint8_t channel, uint8_t r, uint8_t g, uint8_t b) {
        if (channel >= 2) return;  // Solo tenemos 2 canales RGB
        
        // Actualizamos el estado
        state.rgb[channel].r = r;
        state.rgb[channel].g = g;
        state.rgb[channel].b = b;
        
        // Calculamos el canal base (0 para RGB1, 3 para RGB2)
        int baseChannel = (channel == 0) ? RGB1_BASE_CHANNEL : RGB2_BASE_CHANNEL;
        
        // Actualizamos los valores PWM
        pwm.setPWM(baseChannel, 0, convert8to12Bits(r));
        pwm.setPWM(baseChannel + 1, 0, convert8to12Bits(g));
        pwm.setPWM(baseChannel + 2, 0, convert8to12Bits(b));
    }

    // Establece el valor para un canal auxiliar
    void setAuxiliary(uint8_t auxChannel, uint8_t value) {
        if (auxChannel >= 5) return;  // Solo tenemos 5 auxiliares
        
        // Actualizamos el estado
        state.auxiliary[auxChannel] = value;
        
        // Actualizamos el valor PWM
        pwm.setPWM(AUX_BASE_CHANNEL + auxChannel, 0, convert8to12Bits(value));
    }

    // Función para hacer un fundido suave entre dos colores
    void fadeToColor(uint8_t channel, uint8_t targetR, uint8_t targetG, uint8_t targetB, uint16_t duration) {
        uint8_t startR = state.rgb[channel].r;
        uint8_t startG = state.rgb[channel].g;
        uint8_t startB = state.rgb[channel].b;
        
        unsigned long startTime = millis();
        unsigned long elapsedTime;
        float progress;
        
        do {
            elapsedTime = millis() - startTime;
            progress = (float)elapsedTime / duration;
            if (progress > 1.0f) progress = 1.0f;
            
            uint8_t currentR = startR + (targetR - startR) * progress;
            uint8_t currentG = startG + (targetG - startG) * progress;
            uint8_t currentB = startB + (targetB - startB) * progress;
            
            setRGBColor(channel, currentR, currentG, currentB);
            delay(5);  // Pequeña pausa para suavizar la transición
            
        } while (progress < 1.0f);
    }

    // Función para hacer un fundido suave en un auxiliar
    void fadeAuxiliary(uint8_t auxChannel, uint8_t targetValue, uint16_t duration) {
        uint8_t startValue = state.auxiliary[auxChannel];
        
        unsigned long startTime = millis();
        unsigned long elapsedTime;
        float progress;
        
        do {
            elapsedTime = millis() - startTime;
            progress = (float)elapsedTime / duration;
            if (progress > 1.0f) progress = 1.0f;
            
            uint8_t currentValue = startValue + (targetValue - startValue) * progress;
            setAuxiliary(auxChannel, currentValue);
            delay(5);
            
        } while (progress < 1.0f);
    }
};
