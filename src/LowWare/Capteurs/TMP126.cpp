#include "TMP126.h"

// ─────────────────────────────────────────────────────────────────────────────
//  Constructeur & begin
// ─────────────────────────────────────────────────────────────────────────────

TMP126::TMP126(uint8_t csPin, SPIClass &spi, uint32_t spiFreq) : _spi(spi), _csPin(csPin), _spiFreq(spiFreq) {
    // TMP126 : données clocked OUT sur falling SCLK, clocked IN sur rising SCLK
    // → SPI Mode 0 (CPOL=0, CPHA=0), MSB first
    _spiSettings = SPISettings(spiFreq, MSBFIRST, SPI_MODE0);
}

bool TMP126::begin() {
    pinMode(_csPin, OUTPUT);
    digitalWrite(_csPin, HIGH);

    _spi.begin();
    delay(2); // attente POR (tINITIATION ≥ 0.5 ms)

    uint16_t id = readDeviceId();
    // L'ID peut varier selon les révisions ; on vérifie au moins bits[15:4]
    return (id != 0x0000 && id != 0xFFFF);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Reset
// ─────────────────────────────────────────────────────────────────────────────

void TMP126::softReset() {
    uint16_t cfg = readReg(TMP126Reg::CONFIG);
    writeReg(TMP126Reg::CONFIG, cfg | TMP126Config::SOFT_RESET);
    delay(2); // tRESET ≤ 0.5 ms
}

// ─────────────────────────────────────────────────────────────────────────────
//  Configuration
// ─────────────────────────────────────────────────────────────────────────────

void TMP126::setContinuousMode(uint16_t convPeriod, uint16_t averaging) {
    uint16_t cfg = readReg(TMP126Reg::CONFIG);
    // Efface Mode, Conv_Period, AVG
    cfg &= ~((1u << 10) | (7u << 6) | (3u << 4));
    cfg |= TMP126Config::MODE_CONTINUOUS | convPeriod | averaging;
    writeReg(TMP126Reg::CONFIG, cfg);
}

void TMP126::setShutdownMode() {
    uint16_t cfg = readReg(TMP126Reg::CONFIG);
    cfg |= TMP126Config::MODE_SHUTDOWN;
    writeReg(TMP126Reg::CONFIG, cfg);
}

void TMP126::triggerOneShot() {
    uint16_t cfg = readReg(TMP126Reg::CONFIG);
    cfg |= TMP126Config::MODE_SHUTDOWN | TMP126Config::ONE_SHOT;
    writeReg(TMP126Reg::CONFIG, cfg);
}

void TMP126::configureAlert(bool comparatorMode, bool alertActiveHigh, bool dataReadyOnAlert) {
    uint16_t cfg = readReg(TMP126Reg::CONFIG);
    cfg &= ~(TMP126Config::COMP_MODE | TMP126Config::ALERT_POL_HIGH | TMP126Config::DATA_READY_EN);
    if (comparatorMode)
        cfg |= TMP126Config::COMP_MODE;
    if (alertActiveHigh)
        cfg |= TMP126Config::ALERT_POL_HIGH;
    if (dataReadyOnAlert)
        cfg |= TMP126Config::DATA_READY_EN;
    writeReg(TMP126Reg::CONFIG, cfg);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Limites
// ─────────────────────────────────────────────────────────────────────────────

void TMP126::setHighLimit(float tempCelsius) { writeReg(TMP126Reg::THIGH_LIMIT, _tempToRaw(tempCelsius)); }

void TMP126::setLowLimit(float tempCelsius) { writeReg(TMP126Reg::TLOW_LIMIT, _tempToRaw(tempCelsius)); }

void TMP126::setHysteresis(float thighHystCelsius, float tlowHystCelsius) {
    // MSB = THigh_Hyst (bits[15:8]), LSB = TLow_Hyst (bits[7:0])
    // LSB = 0.5°C pour l'hystérésis selon datasheet
    auto toHystRaw = [](float deg) -> uint8_t {
        return static_cast<uint8_t>(constrain(roundf(deg / 0.5f), 0.0f, 255.0f));
    };
    uint16_t val = (static_cast<uint16_t>(toHystRaw(thighHystCelsius)) << 8) |
                   static_cast<uint16_t>(toHystRaw(tlowHystCelsius));
    writeReg(TMP126Reg::HYSTERESIS, val);
}

void TMP126::setSlewLimit(float slewLimitCelsius) {
    // Slew_Limit = unsigned, même LSB que la température : 0.03125 °C
    uint16_t raw = static_cast<uint16_t>(fabsf(slewLimitCelsius) / TMP126_LSB_DEG);
    // Aligner sur bits[15:2] (2 LSBs toujours 0)
    raw = (raw & 0x3FFFu) << 2;
    writeReg(TMP126Reg::SLEW_LIMIT, raw);
}

void TMP126::enableAlerts(bool thigh, bool tlow, bool slew) {
    uint16_t val = 0;
    if (thigh)
        val |= TMP126AlertEn::THIGH_EN;
    if (tlow)
        val |= TMP126AlertEn::TLOW_EN;
    if (slew)
        val |= TMP126AlertEn::SLEW_EN;
    writeReg(TMP126Reg::ALERT_ENABLE, val);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Lectures
// ─────────────────────────────────────────────────────────────────────────────

float TMP126::readTemperature() {
    uint16_t raw = readReg(TMP126Reg::TEMP_RESULT);
    if (raw == 0xFFFF)
        return NAN;
    return _rawToTemp(raw);
}

TMP126AlertStatus TMP126::readAlertStatus() {
    uint16_t reg = readReg(TMP126Reg::ALERT_STATUS);
    TMP126AlertStatus s{};
    s.dataReady = (reg & TMP126Alert::DATA_READY) != 0;
    s.crcError = (reg & TMP126Alert::CRC_FLAG) != 0;
    s.slewFlag = (reg & TMP126Alert::SLEW_FLAG) != 0;
    s.slewStatus = (reg & TMP126Alert::SLEW_STATUS) != 0;
    s.thighFlag = (reg & TMP126Alert::THIGH_FLAG) != 0;
    s.thighStatus = (reg & TMP126Alert::THIGH_STATUS) != 0;
    s.tlowFlag = (reg & TMP126Alert::TLOW_FLAG) != 0;
    s.tlowStatus = (reg & TMP126Alert::TLOW_STATUS) != 0;
    return s;
}

bool TMP126::isDataReady() { return (readReg(TMP126Reg::ALERT_STATUS) & TMP126Alert::DATA_READY) != 0; }

uint16_t TMP126::readDeviceId() { return readReg(TMP126Reg::DEVICE_ID); }

float TMP126::readOneShotBlocking(uint32_t timeoutMs) {
    triggerOneShot();
    uint32_t t0 = millis();
    while (!isDataReady()) {
        if ((millis() - t0) >= timeoutMs)
            return NAN;
        delayMicroseconds(500);
    }
    return readTemperature();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Accès registres bas niveau
// ─────────────────────────────────────────────────────────────────────────────

uint16_t TMP126::readReg(uint8_t reg) {
    uint16_t cmd = _buildCmd(reg, true);

    _spi.beginTransaction(_spiSettings);
    _csLow();
    _transfer16(cmd); // envoi command word, dummy data reçu
    uint16_t data = _transfer16(0x0000); // clock out data word
    _csHigh();
    _spi.endTransaction();

    return data;
}

void TMP126::writeReg(uint8_t reg, uint16_t value) {
    uint16_t cmd = _buildCmd(reg, false);

    _spi.beginTransaction(_spiSettings);
    _csLow();
    _transfer16(cmd); // command word
    _transfer16(value); // data word
    _csHigh();
    _spi.endTransaction();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Privé
// ─────────────────────────────────────────────────────────────────────────────

uint16_t TMP126::_buildCmd(uint8_t reg, bool read, bool autoInc) {
    uint16_t cmd = 0;
    // Bit 15 : don't care → 0
    // Bit 14 : CRC disable → 0
    // Bits[13:10] : CRC block length → 0
    // Bit 9 : Auto Increment
    if (autoInc)
        cmd |= (1u << 9);
    // Bit 8 : R/W
    if (read)
        cmd |= (1u << 8);
    // Bits[7:0] : Sub-Address
    cmd |= static_cast<uint16_t>(reg);
    return cmd;
}

uint16_t TMP126::_transfer16(uint16_t txWord) {
    uint8_t hi = static_cast<uint8_t>(txWord >> 8);
    uint8_t lo = static_cast<uint8_t>(txWord & 0xFF);
    hi = _spi.transfer(hi);
    lo = _spi.transfer(lo);
    return (static_cast<uint16_t>(hi) << 8) | lo;
}

float TMP126::_rawToTemp(uint16_t raw) {
    // Format 14-bit two's complement, bits[15:2], bits[1:0] toujours 00
    int16_t signed_raw = static_cast<int16_t>(raw);
    signed_raw >>= 2; // décalage arithmétique → extension signe
    return static_cast<float>(signed_raw) * TMP126_LSB_DEG;
}

uint16_t TMP126::_tempToRaw(float tempCelsius) {
    int16_t raw14 = static_cast<int16_t>(roundf(tempCelsius / TMP126_LSB_DEG));
    // Remet dans bits[15:2]
    return static_cast<uint16_t>(raw14 << 2);
}

void TMP126::_csLow() {
    digitalWrite(_csPin, LOW);
    delayMicroseconds(1);
}

void TMP126::_csHigh() {
    delayMicroseconds(1);
    digitalWrite(_csPin, HIGH);
    delayMicroseconds(1);
}
