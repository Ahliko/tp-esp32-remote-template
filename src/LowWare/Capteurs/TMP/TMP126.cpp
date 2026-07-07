#include "TMP126.h"

TMP126::TMP126(TMP126Transport &transport) : _transport(transport) {}

bool TMP126::init() const {
    if (!_transport.initBus())
        return false;

    _transport.delayUs(2000);

    const uint16_t id = readDeviceId();
    return id != 0x0000 && id != 0xFFFF;
}

void TMP126::softReset() const {
    const uint16_t cfg = readReg(TMP126Reg::CONFIG);
    writeReg(TMP126Reg::CONFIG, cfg | TMP126Config::SOFT_RESET);
    _transport.delayUs(2000);
}

void TMP126::setContinuousMode(const uint16_t convPeriod, const uint16_t averaging) const {
    uint16_t cfg = readReg(TMP126Reg::CONFIG);
    cfg &= ~(1u << 10 | 7u << 6 | 3u << 4);
    cfg |= TMP126Config::MODE_CONTINUOUS | convPeriod | averaging;
    writeReg(TMP126Reg::CONFIG, cfg);
}

void TMP126::setShutdownMode() const {
    uint16_t cfg = readReg(TMP126Reg::CONFIG);
    cfg |= TMP126Config::MODE_SHUTDOWN;
    writeReg(TMP126Reg::CONFIG, cfg);
}

void TMP126::triggerOneShot() const {
    uint16_t cfg = readReg(TMP126Reg::CONFIG);
    cfg |= TMP126Config::MODE_SHUTDOWN | TMP126Config::ONE_SHOT;
    writeReg(TMP126Reg::CONFIG, cfg);
}

void TMP126::configureAlert(const bool comparatorMode, const bool alertActiveHigh, const bool dataReadyOnAlert) const {
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

void TMP126::setHighLimit(const float tempCelsius) const { writeReg(TMP126Reg::THIGH_LIMIT, tempToRaw(tempCelsius)); }

void TMP126::setLowLimit(const float tempCelsius) const { writeReg(TMP126Reg::TLOW_LIMIT, tempToRaw(tempCelsius)); }

void TMP126::setHysteresis(const float thighHystCelsius, const float tlowHystCelsius) const {
    auto toHystRaw = [](const float deg) -> uint8_t {
        float val = deg / 0.5f;
        if (val < 0.0f)
            val = 0.0f;
        if (val > 255.0f)
            val = 255.0f;
        return static_cast<uint8_t>(val + 0.5f);
    };

    const uint16_t val =
            static_cast<uint16_t>(toHystRaw(thighHystCelsius)) << 8 | static_cast<uint16_t>(toHystRaw(tlowHystCelsius));
    writeReg(TMP126Reg::HYSTERESIS, val);
}

void TMP126::setSlewLimit(const float slewLimitCelsius) const {
    uint16_t raw = static_cast<uint16_t>(std::fabs(slewLimitCelsius) / TMP126_LSB_DEG);
    raw = (raw & 0x3FFFu) << 2;
    writeReg(TMP126Reg::SLEW_LIMIT, raw);
}

void TMP126::enableAlerts(const bool thigh, const bool tlow, const bool slew) const {
    uint16_t val = 0;
    if (thigh)
        val |= TMP126AlertEn::THIGH_EN;
    if (tlow)
        val |= TMP126AlertEn::TLOW_EN;
    if (slew)
        val |= TMP126AlertEn::SLEW_EN;
    writeReg(TMP126Reg::ALERT_ENABLE, val);
}

float TMP126::readTemperature() const {
    const uint16_t raw = readReg(TMP126Reg::TEMP_RESULT);
    if (raw == 0xFFFF)
        return NAN;
    return rawToTemp(raw);
}

TMP126AlertStatus TMP126::readAlertStatus() const { return parseAlertStatus(readReg(TMP126Reg::ALERT_STATUS)); }

bool TMP126::isDataReady() const { return (readReg(TMP126Reg::ALERT_STATUS) & TMP126Alert::DATA_READY) != 0; }

uint16_t TMP126::readDeviceId() const { return readReg(TMP126Reg::DEVICE_ID); }

float TMP126::readOneShotBlocking(const uint32_t timeoutMs) const {
    triggerOneShot();
    const uint32_t t0 = _transport.getMillis();

    while (!isDataReady()) {
        if (_transport.getMillis() - t0 >= timeoutMs)
            return NAN;
        _transport.delayUs(500);
    }
    return readTemperature();
}

uint16_t TMP126::readReg(const uint8_t reg) const { return _transport.readRegRaw(buildCmd(reg, true)); }

void TMP126::writeReg(const uint8_t reg, const uint16_t value) const {
    _transport.writeRegRaw(buildCmd(reg, false), value);
}

uint16_t TMP126::buildCmd(const uint8_t reg, const bool read, const bool autoInc) {
    uint16_t cmd = 0;
    if (autoInc)
        cmd |= 1u << 9;
    if (read)
        cmd |= 1u << 8;
    cmd |= static_cast<uint16_t>(reg);
    return cmd;
}

float TMP126::rawToTemp(const uint16_t raw) {
    int16_t signed_raw = static_cast<int16_t>(raw);
    signed_raw >>= 2;
    return static_cast<float>(signed_raw) * TMP126_LSB_DEG;
}

uint16_t TMP126::tempToRaw(const float tempCelsius) {
    const int16_t raw14 = static_cast<int16_t>(std::round(tempCelsius / TMP126_LSB_DEG));
    return static_cast<uint16_t>(raw14 << 2);
}

TMP126AlertStatus TMP126::parseAlertStatus(uint16_t reg) {
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
