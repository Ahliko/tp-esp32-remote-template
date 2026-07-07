#include "TMP126Low.h"

TMP126Low::TMP126Low(uint8_t csPin, uint32_t spiFreq) : _csPin(csPin), _spiSettings(spiFreq, MSBFIRST, SPI_MODE0) {
    _spi = new SPIClass();
}

TMP126Low::~TMP126Low() { delete _spi; }

bool TMP126Low::initBus() const {
    pinMode(_csPin, OUTPUT);
    digitalWrite(_csPin, HIGH);
    if (_spi == nullptr)
        return false;
    _spi->begin(18, 19, 23, -1);
    return true;
}

uint16_t TMP126Low::readRegRaw(uint16_t cmdWord) const {
    _csLow();
    _spi->beginTransaction(_spiSettings);
    _transfer16(cmdWord);
    uint16_t data = _transfer16(0x0000);
    _spi->endTransaction();
    _csHigh();
    return data;
}

void TMP126Low::writeRegRaw(uint16_t cmdWord, uint16_t dataWord) const {
    _csLow();
    _spi->beginTransaction(_spiSettings);
    _transfer16(cmdWord);
    _transfer16(dataWord);
    _spi->endTransaction();
    _csHigh();
}

uint32_t TMP126Low::getMillis() const { return millis(); }

void TMP126Low::delayUs(uint32_t us) const { delayMicroseconds(us); }

uint16_t TMP126Low::_transfer16(uint16_t txWord) const {
    uint8_t hi = _spi->transfer(txWord >> 8);
    uint8_t lo = _spi->transfer(txWord & 0xFF);
    return (hi << 8) | lo;
}

void TMP126Low::_csLow() const {
    digitalWrite(_csPin, LOW);
    delayMicroseconds(1);
}

void TMP126Low::_csHigh() const {
    delayMicroseconds(1);
    digitalWrite(_csPin, HIGH);
    delayMicroseconds(1);
}
