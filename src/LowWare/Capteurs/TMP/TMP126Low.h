#pragma once
#include <SPI.h>
#include <Arduino.h>
#include "Interface/TMP126Transport.h"

class TMP126Low : public TMP126Transport {
public:
    explicit TMP126Low(uint8_t csPin, uint32_t spiFreq);
    ~TMP126Low() override;

    bool initBus() const override;
    uint16_t readRegRaw(uint16_t cmdWord) const override;
    void writeRegRaw(uint16_t cmdWord, uint16_t dataWord) const override;

    uint32_t getMillis() const override;
    void delayUs(uint32_t us) const override;

private:
    SPIClass* _spi;
    uint8_t _csPin;
    SPISettings _spiSettings;

    void _csLow() const;
    void _csHigh() const;
    uint16_t _transfer16(uint16_t txWord) const;
};