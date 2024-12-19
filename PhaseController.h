#pragma once
#include "SystemState.h"
#include "RGBController.h"
#include <SPIFFS.h>
#include <FS.h>

// Usamos el namespace para File
using fs::File;


class PhaseController {
private:
    // Primero definimos la estructura PhaseConfig que necesitamos para todo lo demás
    struct PhaseConfig {
        // Estructura para RGB
        struct {
            uint8_t r, g, b;
        } rgb[2];  // Array de dos elementos para los dos canales RGB
        
        uint8_t auxiliary[5];  // Cinco canales auxiliares
        bool relay[2];         // Dos relés
        unsigned long duration; // Duración de la fase en milisegundos
        unsigned long crossFade;     // Duración de la transición a la siguiente fase
        
        // Constructor con valores predeterminados
        /*PhaseConfig() : duration(0) {
            for(int i = 0; i < 2; i++) {
                rgb[i] = {0, 0, 0};
            }
            for(int i = 0; i < 5; i++) {
                auxiliary[i] = 0;
            }
            relay[0] = relay[1] = false;
        }*/
        PhaseConfig() : duration(0), crossFade(5000) {  // Por defecto 5 segundos de transición
        for(int i = 0; i < 2; i++) {
            rgb[i] = {0, 0, 0};
        }
        for(int i = 0; i < 5; i++) {
            auxiliary[i] = 0;
        }
        relay[0] = relay[1] = false;
        }
    };

    // Referencias a otros controladores
    SystemState& state;
    RGBController& rgbController;
    
    // Variables de control de secuencia
    bool sequenceRunning = false;
    unsigned long phaseStartTime = 0;
    uint8_t currentPhaseIndex = 0;
    
    // Variables para control de transiciones
    unsigned long transitionStartTime = 0;
    unsigned long transitionDuration = 0;
    uint8_t fromPhase = 0;
    uint8_t toPhase = 0;
    bool inTransition = false;
    
    // Constantes de la secuencia
    static const uint8_t FIRST_SEQUENCE_PHASE = 1;  // Comenzamos desde Alba
    static const uint8_t LAST_SEQUENCE_PHASE = 4;   // Terminamos en Noche
    
    // Almacenamiento de las configuraciones de fase
    PhaseConfig phases[5];  // 5 fases: Apagado, Alba, Día, Tarde, Noche

    // Métodos privados
    void loadDefaultPhases() {
        // Fase 0 - Apagado (ya configurada por el constructor)
        
        // Fase 1 - Alba
        phases[1].rgb[0] = {255, 150, 50};
        phases[1].rgb[1] = {200, 100, 50};
        phases[1].auxiliary[0] = 50;
        
        // Fase 2 - Día
        phases[2].rgb[0] = {255, 255, 255};
        phases[2].rgb[1] = {255, 255, 255};
        phases[2].auxiliary[0] = 255;
        phases[2].auxiliary[1] = 255;
        
        // Fase 3 - Tarde
        phases[3].rgb[0] = {255, 200, 150};
        phases[3].rgb[1] = {255, 180, 120};
        phases[3].auxiliary[0] = 150;
        
        // Fase 4 - Noche
        phases[4].rgb[0] = {50, 50, 150};
        phases[4].rgb[1] = {30, 30, 100};
        phases[4].auxiliary[0] = 20;
    }

    // Función auxiliar para interpolar valores
    uint8_t interpolate(uint8_t start, uint8_t end, float progress) {
        return start + (end - start) * progress;
    }

public:
    const PhaseConfig& getPhaseConfig(uint8_t phase) const {
        return phases[phase];
    }
    // Constructor
    PhaseController(SystemState& systemState, RGBController& rgb)
        : state(systemState), rgbController(rgb) {
        loadDefaultPhases();
    }

    // Métodos de control de secuencia
    void startSequence() {
        sequenceRunning = true;
        currentPhaseIndex = FIRST_SEQUENCE_PHASE;
        phaseStartTime = millis();
        applyPhase(currentPhaseIndex);
        Serial.println("Secuencia iniciada desde fase Alba");
    }

    void stopSequence() {
        sequenceRunning = false;
        Serial.println("Secuencia detenida");
    }

    bool isSequenceRunning() const {
        return sequenceRunning;
    }

    // Métodos de control de fases
    void configurePhase(uint8_t phase, 
                   uint8_t r1, uint8_t g1, uint8_t b1,
                   uint8_t r2, uint8_t g2, uint8_t b2,
                   const uint8_t* auxValues,
                   unsigned long phaseDuration,
                   unsigned long crossFade){
        if (phase >= 5) return;
        
        phases[phase].rgb[0] = {r1, g1, b1};
        phases[phase].rgb[1] = {r2, g2, b2};
        
        for(int i = 0; i < 5; i++) {
            phases[phase].auxiliary[i] = auxValues[i];
        }

        phases[phase].duration = phaseDuration;
        phases[phase].crossFade = crossFade;
        
        Serial.printf("Fase %d configurada: RGB1(%d,%d,%d) RGB2(%d,%d,%d) Duracion:%lu\n Transicion:%lu\n",
                     phase, r1, g1, b1, r2, g2, b2, phaseDuration, crossFade);
    }

    void applyPhase(uint8_t phase) {
        if (phase >= 5) return;
        
        state.currentPhase = phase;
        
        for (int i = 0; i < 2; i++) {
            rgbController.setRGBColor(i, 
                                    phases[phase].rgb[i].r,
                                    phases[phase].rgb[i].g,
                                    phases[phase].rgb[i].b);
        }
        
        for (int i = 0; i < 5; i++) {
            rgbController.setAuxiliary(i, phases[phase].auxiliary[i]);
        }
        
        Serial.printf("Fase %d aplicada\n", phase);
    }

    void startTransition(uint8_t from, uint8_t to, unsigned long duration) {
        if (from >= 5 || to >= 5) return;
        
        fromPhase = from;
        toPhase = to;
        transitionDuration = duration;
        transitionStartTime = millis();
        inTransition = true;
        
        Serial.printf("Iniciando transición de fase %d a fase %d\n", from, to);
    }

    // Método de actualización principal
    void update() {
        static unsigned long lastDebug = 0;
        unsigned long currentTime = millis();

        // Debug cada segundo
        if (currentTime - lastDebug >= 1000) {
            Serial.printf("Estado: Sequence=%d, Transition=%d, Phase=%d, Time=%lu\n",
                         sequenceRunning, inTransition, currentPhaseIndex,
                         (currentTime - phaseStartTime));
            lastDebug = currentTime;
        }

        // Manejo de la secuencia
        if (sequenceRunning) {
            if (currentTime - phaseStartTime >= phases[currentPhaseIndex].duration) {
                uint8_t previousPhase = currentPhaseIndex;
                currentPhaseIndex++;
                if (currentPhaseIndex > LAST_SEQUENCE_PHASE) {
                    currentPhaseIndex = FIRST_SEQUENCE_PHASE;
                }
                phaseStartTime = currentTime;
                startTransition(previousPhase, currentPhaseIndex, 5000);
            }
        }

        // Manejo de transiciones
        if (inTransition) {
            unsigned long elapsedTime = currentTime - transitionStartTime;
            
            if (elapsedTime >= transitionDuration) {
                applyPhase(toPhase);
                inTransition = false;
                Serial.println("Transición completada");
                return;
            }
            
            float progress = (float)elapsedTime / transitionDuration;
            
            for (int i = 0; i < 2; i++) {
                uint8_t r = interpolate(phases[fromPhase].rgb[i].r, phases[toPhase].rgb[i].r, progress);
                uint8_t g = interpolate(phases[fromPhase].rgb[i].g, phases[toPhase].rgb[i].g, progress);
                uint8_t b = interpolate(phases[fromPhase].rgb[i].b, phases[toPhase].rgb[i].b, progress);
                rgbController.setRGBColor(i, r, g, b);
            }
            
            for (int i = 0; i < 5; i++) {
                uint8_t value = interpolate(phases[fromPhase].auxiliary[i], 
                                          phases[toPhase].auxiliary[i], 
                                          progress);
                rgbController.setAuxiliary(i, value);
            }
        }
    }

    void saveToSPIFFS() {
    File file = SPIFFS.open("/phases.cfg", "w");
    if (!file) {
        Serial.println("Error abriendo archivo para escritura");
        return;
    }
    
    file.write((uint8_t*)phases, sizeof(phases));
    file.close();
    Serial.println("Configuración guardada");
}

void loadFromSPIFFS() {
    if (!SPIFFS.exists("/phases.cfg")) {
        Serial.println("No existe archivo de configuración, cargando valores por defecto");
        loadDefaultPhases();
        return;
    }
    
    File file = SPIFFS.open("/phases.cfg", "r");
    if (!file) {
        Serial.println("Error abriendo archivo para lectura");
        return;
    }
    
    file.read((uint8_t*)phases, sizeof(phases));
    file.close();
    Serial.println("Configuración cargada");
    }
    
};
