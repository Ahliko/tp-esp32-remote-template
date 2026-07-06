#include "INA237Low.h"
#include <Arduino.h>

INA237Low::INA237Low(uint8_t i2cAddr, uint8_t sdaPin, uint8_t sclPin) :
    _i2cAddr(i2cAddr), _sdaPin(sdaPin), _sclPin(sclPin) {
    _wire = new TwoWire(0);
}

INA237Low::~INA237Low() { delete _wire; }

bool INA237Low::initBus() const {
    _wire->begin(_sdaPin, _sclPin);
    return true;
}

uint16_t INA237Low::readReg(uint8_t reg) const {
    if (_wire == nullptr)
        return 0xFFFF;
    _wire->beginTransmission(_i2cAddr);
    _wire->write(reg);
    if (_wire->endTransmission(false) != 0)
        return 0xFFFF;

    _wire->requestFrom(_i2cAddr, (uint8_t) 2);
    if (_wire->available() < 2)
        return 0xFFFF;

    uint16_t val = static_cast<uint16_t>(_wire->read()) << 8;
    val |= static_cast<uint16_t>(_wire->read());
    return val;
}

void INA237Low::writeReg(uint8_t reg, uint16_t value) const {
    if (_wire == nullptr)
        return;
    _wire->beginTransmission(_i2cAddr);
    _wire->write(reg);
    _wire->write(static_cast<uint8_t>(value >> 8));
    _wire->write(static_cast<uint8_t>(value & 0xFF));
    _wire->endTransmission();
}

void INA237Low::delayMs(uint32_t ms) const { delay(ms); }
