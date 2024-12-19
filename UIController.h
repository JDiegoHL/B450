#pragma once
#include <TFT_eSPI.h>
#include "SystemState.h"
#include "PhaseController.h"

class UIController {
private:
    TFT_eSPI& tft;
    SystemState& state;
    PhaseController& phaseController;
    
    // Constantes para el layout
    static const int LINE_HEIGHT = 30;
    static const int VALUE_X = 100;
    static const int START_Y = 40;

    // Función auxiliar privada para limpiar áreas
    void clearTextArea(int x, int y, int width, int height) {
        tft.fillRect(x, y, width, height, TFT_BLACK);
    }

    // Nueva implementación unificada de updateValues
    void updateValues() {
        const char* phaseNames[] = {"Apagado", "Alba", "Dia", "Tarde", "Noche"};
        
        // 1. Actualización de fase y tiempos
        clearTextArea(VALUE_X, START_Y, 200, LINE_HEIGHT);
        tft.setTextColor(TFT_MAGENTA, TFT_BLACK);
        tft.setTextSize(2);
        tft.drawString(phaseNames[state.currentPhase], VALUE_X, START_Y);
        
        // Tiempos de la fase
        const auto& currentPhase = phaseController.getPhaseConfig(state.currentPhase);
        char timeStr[32];
        sprintf(timeStr, "%d seg / %d seg", 
                currentPhase.duration/1000,
                currentPhase.crossFade/1000);
        tft.setTextSize(1);
        tft.drawString(timeStr, VALUE_X + 100, START_Y);

        // 2. Valores RGB
        char rgbStr[32];
        tft.setTextSize(2);
        
        // RGB 1
        clearTextArea(VALUE_X, START_Y + LINE_HEIGHT, 200, LINE_HEIGHT);
        sprintf(rgbStr, "R:%3d G:%3d B:%3d", 
                state.rgb[0].r, state.rgb[0].g, state.rgb[0].b);
        tft.setTextColor(TFT_MAGENTA, TFT_BLACK);
        tft.drawString(rgbStr, VALUE_X, START_Y + LINE_HEIGHT);
        
        // RGB 2
        clearTextArea(VALUE_X, START_Y + LINE_HEIGHT * 2, 200, LINE_HEIGHT);
        sprintf(rgbStr, "R:%3d G:%3d B:%3d", 
                state.rgb[1].r, state.rgb[1].g, state.rgb[1].b);
        tft.drawString(rgbStr, VALUE_X, START_Y + LINE_HEIGHT * 2);

        // 3. Auxiliares
        clearTextArea(VALUE_X, START_Y + LINE_HEIGHT * 3, 200, LINE_HEIGHT);
        String auxStatus = "";
        for(int i = 0; i < 5; i++) {
            auxStatus += (i > 0 ? " " : "") + String(state.auxiliary[i]);
        }
        tft.drawString(auxStatus, VALUE_X, START_Y + LINE_HEIGHT * 3);

        // 4. Audio
        clearTextArea(VALUE_X, START_Y + LINE_HEIGHT * 4, 200, LINE_HEIGHT);
        char audioStr[32];
        sprintf(audioStr, "Mode:%d Track:%d", 
                state.audioMode, state.currentTrack);
        tft.drawString(audioStr, VALUE_X, START_Y + LINE_HEIGHT * 4);
    }

public:
    UIController(TFT_eSPI& display, SystemState& systemState, PhaseController& phase)
        : tft(display)
        , state(systemState)
        , phaseController(phase)
    {}

    void begin() {
        tft.init();
        tft.setRotation(1);
        tft.fillScreen(TFT_BLACK);
        drawMainInterface();
    }

    void drawMainInterface() {
        tft.fillScreen(TFT_BLACK);
        
        // Título
        tft.setTextSize(2);
        tft.setTextColor(TFT_CYAN, TFT_BLACK);
        tft.drawString("Control RGB + Audio", 10, 10);
        tft.drawFastHLine(0, 35, tft.width(), TFT_CYAN);
        
        // Etiquetas fijas
        tft.setTextSize(2);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.drawString("Fase:", 10, START_Y);
        tft.drawString("RGB 1:", 10, START_Y + LINE_HEIGHT);
        tft.drawString("RGB 2:", 10, START_Y + LINE_HEIGHT * 2);
        tft.drawString("Aux:", 10, START_Y + LINE_HEIGHT * 3);
        tft.drawString("Audio:", 10, START_Y + LINE_HEIGHT * 4);
        
        updateValues();  // Actualizamos los valores iniciales
    }

    void updateDisplay() {
        static unsigned long lastUpdate = 0;
        const unsigned long UPDATE_INTERVAL = 100;
        
        if (millis() - lastUpdate < UPDATE_INTERVAL) {
            return;
        }
        lastUpdate = millis();

        static uint8_t lastPhase = 255;
        if (state.currentPhase != lastPhase) {
            drawMainInterface();
            lastPhase = state.currentPhase;
        } else {
            updateValues();
        }
    }
};
