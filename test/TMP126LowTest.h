#pragma once
#include <map>
#include <thread>
#include <chrono>

#include "Interface/TMP126Transport.h"

class TMP126LowTest : public TMP126Transport {
public:
    mutable std::map<uint16_t, uint16_t> registers;
    mutable uint32_t simulatedMillis = 0;

    TMP126LowTest() {
        registers[0x0C] = 0x1126; // DEVICE_ID par défaut
        registers[0x00] = 0x0000; // Temp = 0°C par defaut
        registers[0x01] = 0x0000; // Alert Status
    }

    bool initBus() const override { return true; }

    uint16_t readRegRaw(uint16_t cmdWord) const override {
        uint8_t regAddr = cmdWord & 0xFF;
        return registers[regAddr];
    }

    void writeRegRaw(uint16_t cmdWord, uint16_t dataWord) const override {
        uint8_t regAddr = cmdWord & 0xFF;
        registers[regAddr] = dataWord;
    }

    uint32_t getMillis() const override {
        return simulatedMillis++; // Incrémente à chaque appel pour éviter les boucles infinies en test
    }

    void delayUs(uint32_t us) const override {
        // En test on ne bloque pas vraiment, on peut avancer l'horloge simulée
        simulatedMillis += (us / 1000);
    }
    
    // -- Utilitaires de Test --
    
    void simulateTemperatureRaw(uint16_t rawTemp) {
        registers[0x00] = rawTemp;
    }
    
    void setReadyFlag() {
        // Bit 9 (Data_Ready) du registre ALERT_STATUS (0x01)
        registers[0x01] |= (1u << 9);
    }
};