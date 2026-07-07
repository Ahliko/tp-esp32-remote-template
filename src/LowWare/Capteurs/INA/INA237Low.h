#pragma once
#include <Wire.h>

#include "Interface/INA237Transport.h"

class INA237Low : public INA237Transport {
public:
    explicit INA237Low(uint8_t i2cAddr, uint8_t sdaPin = 21, uint8_t sclPin = 22);
    ~INA237Low() override;

    bool initBus() const override;
    uint16_t readReg(uint8_t reg) const override;
    void writeReg(uint8_t reg, uint16_t value) const override;
    void delayMs(uint32_t ms) const override;

private:
    TwoWire *_wire;
    uint8_t _i2cAddr;
    uint8_t _sdaPin;
    uint8_t _sclPin;
};
