#pragma once
#include "../src/Interface/INA237Transport.h"
#include <map>

class INA237LowTest : public INA237Transport {
public:
    mutable std::map<uint8_t, uint16_t> registers;

    INA237LowTest() {
        registers[0x3E] = 0x5449; // MANUFACTURER_ID
        registers[0x04] = 0x0000; // VSHUNT = 0
        registers[0x05] = 0x0000; // VBUS = 0
        registers[0x06] = 0x0000; // DIETEMP = 0
        registers[0x07] = 0x0000; // CURRENT = 0
        registers[0x08] = 0x0000; // POWER = 0
    }

    bool initBus() const override { return true; }

    uint16_t readReg(uint8_t reg) const override {
        return registers[reg];
    }

    void writeReg(uint8_t reg, uint16_t value) const override {
        registers[reg] = value;
    }

    void delayMs(uint32_t) const override { /* Rien en test */ }
    
    // Utilitaires de simulation
    void simulateBusVoltageRaw(uint16_t raw) { registers[0x05] = raw; }
    void simulateCurrentRaw(uint16_t raw) { registers[0x07] = raw; }
    void simulateTemperatureRaw(uint16_t raw) { registers[0x06] = raw; }
};