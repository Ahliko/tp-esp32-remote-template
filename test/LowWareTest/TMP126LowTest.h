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
        registers[0x0C] = 0x1126;
        registers[0x00] = 0x0000;
        registers[0x01] = 0x0000;
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
        return simulatedMillis++;
    }

    void delayUs(uint32_t us) const override {
        simulatedMillis += (us / 1000);
    }

    void simulateTemperatureRaw(uint16_t rawTemp) {
        registers[0x00] = rawTemp;
    }

    void setReadyFlag() {
        registers[0x01] |= (1u << 9);
    }
};