//BluetoothController.h
#pragma once
#include <BluetoothSerial.h>
#include "SystemState.h"
#include "RGBController.h"
#include "AudioController.h"
#include "PhaseController.h"  

class BluetoothController {
private:
    BluetoothSerial SerialBT;
    SystemState& state;
    RGBController& rgbController;
    AudioController& audioController;
    PhaseController& phaseController;  // Añadimos esta referencia
    
    // Buffer para comandos
    static const int MAX_COMMAND_LENGTH = 64;
    char commandBuffer[MAX_COMMAND_LENGTH];
    int bufferIndex = 0;

public:
    // Modificamos el constructor para incluir PhaseController
    BluetoothController(SystemState& systemState,
                       RGBController& rgb,
                       AudioController& audio,
                       PhaseController& phase)
        : state(systemState)
        , rgbController(rgb)
        , audioController(audio)
        , phaseController(phase)
    {
    }
    
    void begin() {
        SerialBT.begin("Better_Controller");
    }
    
    /*void processCommand(const String& command) {
        // Dividimos el comando en partes
        int commaIndex = command.indexOf(',');
        if (commaIndex == -1) return;
        
        String cmd = command.substring(0, commaIndex);
        String params = command.substring(commaIndex + 1);
        
        if (cmd == "FASE") {
            // FASE,<número>
            int fase = params.toInt();
            if (fase >= 0 && fase < 5) {
                phaseController.applyPhase(fase);
                SerialBT.println("Fase aplicada: " + String(fase));
            }
        }
        else if (cmd == "TRANSICION") {
            // TRANSICION,<origen>,<destino>,<duración>
            int comma1 = params.indexOf(',');
            int comma2 = params.indexOf(',', comma1 + 1);
            if (comma1 == -1 || comma2 == -1) return;
            
            int faseOrigen = params.substring(0, comma1).toInt();
            int faseDestino = params.substring(comma1 + 1, comma2).toInt();
            int duracion = params.substring(comma2 + 1).toInt();
            
            if (faseOrigen >= 0 && faseOrigen < 5 && 
                faseDestino >= 0 && faseDestino < 5) {
                phaseController.startTransition(faseOrigen, faseDestino, duracion);
                SerialBT.println("Iniciando transición");
            }
        }
    }*/
    void processCommand(const String& command) {
    // Dividimos el comando en partes
    /*int commaIndex = command.indexOf(',');
    if (commaIndex == -1) return;
    
    String cmd = command.substring(0, commaIndex);
    String params = command.substring(commaIndex + 1);*/
    int firstComma = command.indexOf(',');
    if (firstComma == -1) return;
    
    String cmd = command.substring(0, firstComma);
    String params = command.substring(firstComma + 1);

    //String cmd;
    
    /*if (firstComma == -1) {
        // Si no hay coma, el comando completo es el nombre
        cmd = command;
    } else {
        cmd = command.substring(0, firstComma);
    }*/
    
    // Eliminamos espacios en blanco
    //cmd.trim();
    //if (firstComma == -1) return;
    
    //String cmd = command.substring(0, firstComma);
    //String params = command.substring(firstComma + 1);  // Definimos params aquí
    
    if (cmd == "FASE") {
        // FASE,<número>
        int fase = params.toInt();
        if (fase >= 0 && fase < 5) {
            phaseController.applyPhase(fase);
            SerialBT.println("Fase aplicada: " + String(fase));
        }
    }
    else if (cmd == "TRANSICION") {
        // TRANSICION,<origen>,<destino>,<duración>
        int comma1 = params.indexOf(',');
        int comma2 = params.indexOf(',', comma1 + 1);
        if (comma1 == -1 || comma2 == -1) return;
        
        int faseOrigen = params.substring(0, comma1).toInt();
        int faseDestino = params.substring(comma1 + 1, comma2).toInt();
        int duracion = params.substring(comma2 + 1).toInt();
        
        if (faseOrigen >= 0 && faseOrigen < 5 && 
            faseDestino >= 0 && faseDestino < 5) {
            phaseController.startTransition(faseOrigen, faseDestino, duracion);
            SerialBT.println("Iniciando transición");
        }
    }
    else if (cmd == "CONFIG_FASE") {
        // Creamos un array para almacenar todos los valores
        int valores[14];
        int index = 0;
        String tempParams = params;
        
        // Procesamos cada valor separado por comas
        while (index < 14 && tempParams.length() > 0) {
            int nextComma = tempParams.indexOf(',');
            if (nextComma == -1) {
                // Último valor
                valores[index++] = tempParams.toInt();
                break;
            }
            String valorStr = tempParams.substring(0, nextComma);
            valores[index++] = valorStr.toInt();
            tempParams = tempParams.substring(nextComma + 1);
        }
        
        // Verificamos que tenemos todos los valores
        /*if (index == 13) {
            uint8_t auxValues[5] = {
                (uint8_t)valores[7], 
                (uint8_t)valores[8], 
                (uint8_t)valores[9], 
                (uint8_t)valores[10], 
                (uint8_t)valores[11]
            };
            
            // Añadimos debug para verificar los valores
            SerialBT.println("Configurando fase:");
            SerialBT.printf("Fase: %d\n", valores[0]);
            SerialBT.printf("RGB1: %d,%d,%d\n", valores[1], valores[2], valores[3]);
            SerialBT.printf("RGB2: %d,%d,%d\n", valores[4], valores[5], valores[6]);
            
            phaseController.configurePhase(
                valores[0],                    // fase
                valores[1], valores[2], valores[3],  // RGB1
                valores[4], valores[5], valores[6],  // RGB2
                auxValues,                          // auxiliares
                valores[12]                         // duración
            );
        }*/ 
        
        if (index == 14) {  // Ahora esperamos 14 valores (13 anteriores + tiempo de transición)
        uint8_t auxValues[5] = {
            (uint8_t)valores[7], 
            (uint8_t)valores[8], 
            (uint8_t)valores[9], 
            (uint8_t)valores[10], 
            (uint8_t)valores[11]
        };
            SerialBT.println("Configurando fase:");
            SerialBT.printf("Fase: %d\n", valores[0]);
            SerialBT.printf("RGB1: %d,%d,%d\n", valores[1], valores[2], valores[3]);
            SerialBT.printf("RGB2: %d,%d,%d\n", valores[4], valores[5], valores[6]);
            //SerialBT.printf("T: %d\n", valores[12]/1000);
        
        phaseController.configurePhase(
            (uint8_t)valores[0],                    // fase
            (uint8_t)valores[1],                    // r1
            (uint8_t)valores[2],                    // g1
            (uint8_t)valores[3],                    // b1
            (uint8_t)valores[4],                    // r2
            (uint8_t)valores[5],                    // g2
            (uint8_t)valores[6],                    // b2
            auxValues,                              // auxiliares
            (unsigned long)valores[12],             // duración
            (unsigned long)valores[13]              // tiempo de transición
        );
        
        // Guardamos la configuración después de cada cambio
        phaseController.saveToSPIFFS();
        SerialBT.println("Fase configurada y guardada");
    }else {
            SerialBT.println("Error: Número incorrecto de parámetros");
        }
    }
    else if (cmd == "PLAY") {
        Serial.println("Comando PLAY recibido");  // Debug
        phaseController.startSequence();
        SerialBT.println("Iniciando secuencia");
        Serial.printf("Estado de secuencia: %s\n", 
                     phaseController.isSequenceRunning() ? "Activa" : "Inactiva");
    }
    else if (cmd == "STOP") {
        phaseController.stopSequence();
        SerialBT.println("Secuencia detenida");
    }
}
    
    void update() {
        // Lee comandos Bluetooth
        while (SerialBT.available()) {
            char c = SerialBT.read();
            
            if (c == '\n' || c == '\r') {
                if (bufferIndex > 0) {
                    commandBuffer[bufferIndex] = '\0';
                    processCommand(String(commandBuffer));
                    bufferIndex = 0;
                }
            } else if (bufferIndex < MAX_COMMAND_LENGTH - 1) {
                commandBuffer[bufferIndex++] = c;
            }
        }
    }
};
