#include <TFT_eSPI.h>
#include <BluetoothSerial.h>
#include <Adafruit_PWMServoDriver.h>
#include <SPIFFS.h>
#include <Wire.h>
#include <SPI.h>
#include <FS.h>

#include "SystemState.h"
#include "RGBController.h"
#include "AudioController.h"
#include "BluetoothController.h"
#include "UIController.h"
#include "PhaseController.h"

// Instancias principales
TFT_eSPI tft;
Adafruit_PWMServoDriver pwm;
SystemState systemState;


// Instacias
RGBController rgbController(pwm, systemState);
AudioController audioController(systemState);
PhaseController phaseController(systemState, rgbController);
//UIController uiController(tft, systemState);

UIController uiController(tft, systemState, phaseController);

BluetoothController btController(systemState, rgbController, audioController, phaseController);

void setup() {
    Serial.begin(115200);
    Serial.println("Iniciando sistema...");

    if (!SPIFFS.begin(true)) {
        Serial.println("Error al montar SPIFFS");
        return;
    }

    Wire.begin(21, 22);
    phaseController.loadFromSPIFFS();
    rgbController.begin();
    audioController.begin();
    uiController.begin();
    btController.begin();
    
    Serial.println("Sistema iniciado correctamente");
}

void loop() {
    static unsigned long lastDebug = 0;
    
    btController.update();
    phaseController.update();
    uiController.updateDisplay();

    // Debug periódico
    if (millis() - lastDebug > 2000) {
        Serial.println("Loop ejecutándose...");
        lastDebug = millis();
    }
    static unsigned long lastDisplayUpdate = 0;
    if (millis() - lastDisplayUpdate >= 100) {  // Actualizar cada 100ms
        uiController.updateDisplay();
        lastDisplayUpdate = millis();
    }
    
}



void testRGBAndAuxiliaries() {
    Serial.println("Iniciando prueba de LEDs RGB y auxiliares...");
    
    // Prueba de canales RGB individuales
    Serial.println("Probando RGB1 - Rojo");
    rgbController.setRGBColor(0, 255, 0, 0);
    delay(1000);
    
    Serial.println("Probando RGB1 - Verde");
    rgbController.setRGBColor(0, 0, 255, 0);
    delay(1000);
    
    Serial.println("Probando RGB1 - Azul");
    rgbController.setRGBColor(0, 0, 0, 255);
    delay(1000);
    
    // Prueba de fundido en RGB1
    Serial.println("Probando fundido RGB1");
    rgbController.fadeToColor(0, 255, 255, 0, 2000);  // Amarillo
    delay(1000);
    
    // Repetir para RGB2
    Serial.println("Probando RGB2 - Secuencia similar");
    rgbController.setRGBColor(1, 255, 0, 0);
    delay(1000);
    rgbController.fadeToColor(1, 0, 0, 255, 2000);
    delay(1000);
    
    // Prueba de auxiliares
    Serial.println("Probando auxiliares");
    for (int i = 0; i < 5; i++) {
        Serial.printf("Auxiliar %d - Rampa ascendente\n", i + 1);
        rgbController.fadeAuxiliary(i, 255, 1000);
        delay(500);
        Serial.printf("Auxiliar %d - Rampa descendente\n", i + 1);
        rgbController.fadeAuxiliary(i, 0, 1000);
        delay(500);
    }
    
    // Prueba final: todo encendido y apagado
    Serial.println("Prueba final: Todo ON/OFF");
    // Encender todo
    rgbController.setRGBColor(0, 255, 255, 255);
    rgbController.setRGBColor(1, 255, 255, 255);
    for (int i = 0; i < 5; i++) {
        rgbController.setAuxiliary(i, 255);
    }
    delay(2000);
    
    // Apagar todo
    rgbController.setRGBColor(0, 0, 0, 0);
    rgbController.setRGBColor(1, 0, 0, 0);
    for (int i = 0; i < 5; i++) {
        rgbController.setAuxiliary(i, 0);
    }
    
    Serial.println("Prueba completada");
}
