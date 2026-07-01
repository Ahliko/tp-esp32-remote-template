#include "INA237.h"
#include <Arduino.h>

#include "Config/config.h"

// ─────────────────────────────────────────────────────────────────────────────
//  Constantes internes
// ─────────────────────────────────────────────────────────────────────────────
static constexpr uint16_t MANUFACTURER_ID_EXPECTED = 0x5449; // 'TI'
static constexpr uint16_t DEVICE_ID_EXPECTED = 0x2370;

// Bits CONFIG (reg 0x00)
static constexpr uint16_t CONFIG_RST_BIT = 1u << 15;
static constexpr uint16_t CONFIG_ADCRANGE_BIT = 1u << 4;

static constexpr uint16_t DIAG_CNVRF = 1u << 1;

INA237::INA237() { _wire = new TwoWire(INAADDR); }

INA237::~INA237() {
    delete _wire;
}

bool INA237::init(const ADCRange range) {
    _shuntOhms = ShuntOhms;
    _range = range;

    // Vérification de l'ID fabricant
    if (readManufacturerId() != MANUFACTURER_ID_EXPECTED)
        return false;
    if (readDeviceId() >> 4 != DEVICE_ID_EXPECTED >> 4)
        return false;

    reset();
    delay(2); // attente stabilisation

    // CONFIG : ADCRANGE
    uint16_t cfg = 0;
    if (range == ADCRange::RANGE_40_96mV)
        cfg |= CONFIG_ADCRANGE_BIT;
    writeReg(INA237Reg::CONFIG, cfg);

    // ADC_CONFIG : continu, shunt+bus+temp, 1052µs, avg=1
    constexpr uint16_t adcCfg = static_cast<uint16_t>(OperatingMode::CONT_ALL) << 12 |
                                static_cast<uint16_t>(ConvTime::US_1052) << 9 // VTCT
                                | static_cast<uint16_t>(ConvTime::US_1052) << 6 // VSHCT
                                | static_cast<uint16_t>(ConvTime::US_1052) << 3 // VBUSCT
                                | static_cast<uint16_t>(Averaging::AVG_1);
    writeReg(INA237Reg::ADC_CONFIG, adcCfg);

    calibrate(ShuntOhms, MaxCurrentA);
    return true;
}

void INA237::reset() const { writeReg(INA237Reg::CONFIG, CONFIG_RST_BIT); }

void INA237::setMode(OperatingMode mode) const {
    uint16_t reg = readReg(INA237Reg::ADC_CONFIG);
    reg = reg & 0x0FFFu | static_cast<uint16_t>(mode) << 12;
    writeReg(INA237Reg::ADC_CONFIG, reg);
}

void INA237::setShuntConvTime(ConvTime ct) const {
    uint16_t reg = readReg(INA237Reg::ADC_CONFIG);
    reg = reg & ~(0x7u << 6) | static_cast<uint16_t>(ct) << 6;
    writeReg(INA237Reg::ADC_CONFIG, reg);
}

void INA237::setBusConvTime(ConvTime ct) const {
    uint16_t reg = readReg(INA237Reg::ADC_CONFIG);
    reg = reg & ~(0x7u << 3) | static_cast<uint16_t>(ct) << 3;
    writeReg(INA237Reg::ADC_CONFIG, reg);
}

void INA237::setTempConvTime(ConvTime ct) const {
    uint16_t reg = readReg(INA237Reg::ADC_CONFIG);
    reg = reg & ~(0x7u << 9) | static_cast<uint16_t>(ct) << 9;
    writeReg(INA237Reg::ADC_CONFIG, reg);
}

void INA237::setAveraging(Averaging avg) const {
    uint16_t reg = readReg(INA237Reg::ADC_CONFIG);
    reg = reg & ~0x7u | static_cast<uint16_t>(avg);
    writeReg(INA237Reg::ADC_CONFIG, reg);
}

void INA237::setConversionDelay(const uint8_t delay2ms) const {
    uint16_t reg = readReg(INA237Reg::CONFIG);
    reg = reg & ~(0xFFu << 6) | static_cast<uint16_t>(delay2ms) << 6;
    writeReg(INA237Reg::CONFIG, reg);
}

void INA237::setShuntTempCoeff(const uint16_t ppmPerCelsius) const {
    // SHUNT_TEMPCO reg 0x03 : bits 13:0
    writeReg(INA237Reg::SHUNT_TEMPCO, ppmPerCelsius & 0x3FFFu);
}

// ── Lectures ─────────────────────────────────────────────────────────────────

float INA237::readShuntVoltage() const {
    const auto raw = static_cast<int16_t>(readReg(INA237Reg::VSHUNT));
    // LSB : 5µV (ADCRANGE=0) ou 1.25µV (ADCRANGE=1)
    return static_cast<float>(raw) * shuntLSB_uV() * 1e-6f;
}

float INA237::readBusVoltage() const {
    const auto raw = static_cast<int16_t>(readReg(INA237Reg::VBUS));
    // LSB = 3.125 mV
    return static_cast<float>(raw) * 3.125e-3f;
}

float INA237::readTemperature() const {
    auto raw = static_cast<int16_t>(readReg(INA237Reg::DIETEMP));
    // Bits 15:4, LSB = 125 m°C → shift droite de 4
    raw >>= 4;
    return static_cast<float>(raw) * 0.125f;
}

float INA237::readCurrent() const {
    const auto raw = static_cast<int16_t>(readReg(INA237Reg::CURRENT));
    return static_cast<float>(raw) * _currentLSB;
}

float INA237::readPower() const {
    // POWER : registre 24 bits non signé, stocké sur 3 octets → ici 16 bits MSB
    // Selon datasheet : registre 16 bits, LSB = 3.2 × CurrentLSB
    const uint16_t raw = readReg(INA237Reg::POWER);
    return static_cast<float>(raw) * 3.2f * _currentLSB;
}


bool INA237::isConversionReady() const { return (readReg(0x0F) & DIAG_CNVRF) != 0; }

uint16_t INA237::readManufacturerId() const { return readReg(INA237Reg::MANUFACTURER_ID); }

uint16_t INA237::readDeviceId() const { return readReg(INA237Reg::DEVICE_ID); }

// ── Privé ────────────────────────────────────────────────────────────────────

void INA237::calibrate(const float shuntOhms, const float maxCurrentA) {
    // currentLSB = MaxExpectedCurrent / 2^15
    _currentLSB = maxCurrentA / 32768.0f;

    // SHUNT_CAL = 13107.2e6 × CurrentLSB × Rshunt
    //   (facteur de la datasheet p.21 : 13107.2 × 10^6 pour plage ±163.84mV)
    //   Pour plage ±40.96mV le facteur est ×4 : 52428.8 × 10^6
    const float factor = _range == ADCRange::RANGE_163_84mV ? 13107.2e6f : 52428.8e6f;
    const auto shuntCal = static_cast<uint16_t>(factor * _currentLSB * shuntOhms);
    writeReg(INA237Reg::SHUNT_CAL, shuntCal);
}

uint16_t INA237::readReg(const uint8_t reg) const {
    if (_wire == nullptr) return 0xFFFF;
    _wire->beginTransmission(INAADDR);
    _wire->write(reg);
    if (_wire->endTransmission(false) != 0)
        return 0xFFFF;
    _wire->requestFrom(static_cast<uint8_t>(INAADDR), static_cast<uint8_t>(2));
    if (_wire->available() < 2)
        return 0xFFFF;
    uint16_t val = static_cast<uint16_t>(_wire->read()) << 8;
    val |= static_cast<uint16_t>(_wire->read());
    return val;
}

void INA237::writeReg(const uint8_t reg, const uint16_t value) const {
    if (_wire == nullptr) return;
    _wire->beginTransmission(INAADDR);
    _wire->write(reg);
    _wire->write(static_cast<uint8_t>(value >> 8 & 0xFF));
    _wire->write(static_cast<uint8_t>(value & 0xFF));
    _wire->endTransmission();
}
