#include "INA237.h"
#include <Arduino.h>

// ─────────────────────────────────────────────────────────────────────────────
//  Constantes internes
// ─────────────────────────────────────────────────────────────────────────────
static constexpr uint16_t MANUFACTURER_ID_EXPECTED = 0x5449; // 'TI'
static constexpr uint16_t DEVICE_ID_EXPECTED = 0x2370;

// Bits CONFIG (reg 0x00)
static constexpr uint16_t CONFIG_RST_BIT = (1u << 15);
static constexpr uint16_t CONFIG_ADCRANGE_BIT = (1u << 4);

// Bits DIAG_ALRT (reg 0x0F)
static constexpr uint16_t DIAG_ALATCH = (1u << 15);
static constexpr uint16_t DIAG_CNVR = (1u << 14);
static constexpr uint16_t DIAG_SLOWALERT = (1u << 13);
static constexpr uint16_t DIAG_APOL = (1u << 12);
static constexpr uint16_t DIAG_MATHOF = (1u << 9);
static constexpr uint16_t DIAG_TMPOL = (1u << 7);
static constexpr uint16_t DIAG_SHNTOL = (1u << 6);
static constexpr uint16_t DIAG_SHNTUL = (1u << 5);
static constexpr uint16_t DIAG_BUSOL = (1u << 4);
static constexpr uint16_t DIAG_BUSUL = (1u << 3);
static constexpr uint16_t DIAG_POL = (1u << 2);
static constexpr uint16_t DIAG_CNVRF = (1u << 1);
static constexpr uint16_t DIAG_MEMSTAT = (1u << 0);

// ─────────────────────────────────────────────────────────────────────────────
//  Implémentation
// ─────────────────────────────────────────────────────────────────────────────

INA237::INA237(TwoWire &wire) : _wire(wire) {}

bool INA237::begin(float shuntOhms, float maxCurrentA, ADCRange range) {
    _shuntOhms = shuntOhms;
    _range = range;

    // Vérification de l'ID fabricant
    if (readManufacturerId() != MANUFACTURER_ID_EXPECTED)
        return false;
    if ((readDeviceId() >> 4) != (DEVICE_ID_EXPECTED >> 4))
        return false;

    reset();
    delay(2); // attente stabilisation

    // CONFIG : ADCRANGE
    uint16_t cfg = 0;
    if (range == ADCRange::RANGE_40_96mV)
        cfg |= CONFIG_ADCRANGE_BIT;
    writeReg(INA237Reg::CONFIG, cfg);

    // ADC_CONFIG : continu, shunt+bus+temp, 1052µs, avg=1
    uint16_t adcCfg = (static_cast<uint16_t>(OperatingMode::CONT_ALL) << 12) |
                      (static_cast<uint16_t>(ConvTime::US_1052) << 9) // VTCT
                      | (static_cast<uint16_t>(ConvTime::US_1052) << 6) // VSHCT
                      | (static_cast<uint16_t>(ConvTime::US_1052) << 3) // VBUSCT
                      | (static_cast<uint16_t>(Averaging::AVG_1));
    writeReg(INA237Reg::ADC_CONFIG, adcCfg);

    calibrate(shuntOhms, maxCurrentA);
    return true;
}

void INA237::reset() { writeReg(INA237Reg::CONFIG, CONFIG_RST_BIT); }

void INA237::setMode(OperatingMode mode) {
    uint16_t reg = readReg(INA237Reg::ADC_CONFIG);
    reg = (reg & 0x0FFFu) | (static_cast<uint16_t>(mode) << 12);
    writeReg(INA237Reg::ADC_CONFIG, reg);
}

void INA237::setShuntConvTime(ConvTime ct) {
    uint16_t reg = readReg(INA237Reg::ADC_CONFIG);
    reg = (reg & ~(0x7u << 6)) | (static_cast<uint16_t>(ct) << 6);
    writeReg(INA237Reg::ADC_CONFIG, reg);
}

void INA237::setBusConvTime(ConvTime ct) {
    uint16_t reg = readReg(INA237Reg::ADC_CONFIG);
    reg = (reg & ~(0x7u << 3)) | (static_cast<uint16_t>(ct) << 3);
    writeReg(INA237Reg::ADC_CONFIG, reg);
}

void INA237::setTempConvTime(ConvTime ct) {
    uint16_t reg = readReg(INA237Reg::ADC_CONFIG);
    reg = (reg & ~(0x7u << 9)) | (static_cast<uint16_t>(ct) << 9);
    writeReg(INA237Reg::ADC_CONFIG, reg);
}

void INA237::setAveraging(Averaging avg) {
    uint16_t reg = readReg(INA237Reg::ADC_CONFIG);
    reg = (reg & ~0x7u) | static_cast<uint16_t>(avg);
    writeReg(INA237Reg::ADC_CONFIG, reg);
}

void INA237::setConversionDelay(uint8_t delay2ms) {
    uint16_t reg = readReg(INA237Reg::CONFIG);
    reg = (reg & ~(0xFFu << 6)) | (static_cast<uint16_t>(delay2ms) << 6);
    writeReg(INA237Reg::CONFIG, reg);
}

void INA237::setShuntTempCoeff(uint16_t ppmPerCelsius) {
    // SHUNT_TEMPCO reg 0x03 : bits 13:0
    writeReg(INA237Reg::SHUNT_TEMPCO, ppmPerCelsius & 0x3FFFu);
}

// ── Lectures ─────────────────────────────────────────────────────────────────

float INA237::readShuntVoltage() {
    int16_t raw = static_cast<int16_t>(readReg(INA237Reg::VSHUNT));
    // LSB : 5µV (ADCRANGE=0) ou 1.25µV (ADCRANGE=1)
    return static_cast<float>(raw) * shuntLSB_uV() * 1e-6f;
}

float INA237::readBusVoltage() {
    int16_t raw = static_cast<int16_t>(readReg(INA237Reg::VBUS));
    // LSB = 3.125 mV
    return static_cast<float>(raw) * 3.125e-3f;
}

float INA237::readTemperature() {
    int16_t raw = static_cast<int16_t>(readReg(INA237Reg::DIETEMP));
    // Bits 15:4, LSB = 125 m°C → shift droite de 4
    raw >>= 4;
    return static_cast<float>(raw) * 0.125f;
}

float INA237::readCurrent() {
    int16_t raw = static_cast<int16_t>(readReg(INA237Reg::CURRENT));
    return static_cast<float>(raw) * _currentLSB;
}

float INA237::readPower() {
    // POWER : registre 24 bits non signé, stocké sur 3 octets → ici 16 bits MSB
    // Selon datasheet : registre 16 bits, LSB = 3.2 × CurrentLSB
    uint16_t raw = readReg(INA237Reg::POWER);
    return static_cast<float>(raw) * 3.2f * _currentLSB;
}

// ── Alertes ──────────────────────────────────────────────────────────────────

void INA237::setShuntOverVoltageLimit(float voltLimit) {
    int16_t raw = static_cast<int16_t>(voltLimit / (shuntLSB_uV() * 1e-6f));
    writeReg(INA237Reg::SOVL, static_cast<uint16_t>(raw));
}

void INA237::setShuntUnderVoltageLimit(float voltLimit) {
    int16_t raw = static_cast<int16_t>(voltLimit / (shuntLSB_uV() * 1e-6f));
    writeReg(INA237Reg::SUVL, static_cast<uint16_t>(raw));
}

void INA237::setBusOverVoltageLimit(float voltLimit) {
    int16_t raw = static_cast<int16_t>(voltLimit / 3.125e-3f);
    writeReg(INA237Reg::BOVL, static_cast<uint16_t>(raw));
}

void INA237::setBusUnderVoltageLimit(float voltLimit) {
    int16_t raw = static_cast<int16_t>(voltLimit / 3.125e-3f);
    writeReg(INA237Reg::BUVL, static_cast<uint16_t>(raw));
}

void INA237::setTemperatureLimit(float tempCelsius) {
    int16_t raw = static_cast<int16_t>(tempCelsius / 0.125f) << 4;
    writeReg(INA237Reg::TEMP_LIMIT, static_cast<uint16_t>(raw));
}

void INA237::setPowerLimit(float wattLimit) {
    uint16_t raw = static_cast<uint16_t>(wattLimit / (3.2f * _currentLSB));
    writeReg(INA237Reg::PWR_LIMIT, raw);
}

DiagAlert INA237::readDiagAlert() {
    uint16_t reg = readReg(0x0F);
    DiagAlert d{};
    d.memstat = (reg & DIAG_MEMSTAT) != 0;
    d.cnvrf = (reg & DIAG_CNVRF) != 0;
    d.pol = (reg & DIAG_POL) != 0;
    d.busul = (reg & DIAG_BUSUL) != 0;
    d.busol = (reg & DIAG_BUSOL) != 0;
    d.shntul = (reg & DIAG_SHNTUL) != 0;
    d.shntol = (reg & DIAG_SHNTOL) != 0;
    d.tmpol = (reg & DIAG_TMPOL) != 0;
    d.mathof = (reg & DIAG_MATHOF) != 0;
    d.slowalert = (reg & DIAG_SLOWALERT) != 0;
    d.apol = (reg & DIAG_APOL) != 0;
    d.cnvr = (reg & DIAG_CNVR) != 0;
    d.alatch = (reg & DIAG_ALATCH) != 0;
    return d;
}

void INA237::configureAlert(bool latch, bool invertPolarity, bool convReadyEnable, bool slowAlert) {
    uint16_t reg = readReg(0x0F);
    reg &= 0x0FFFu; // clear config bits
    if (latch)
        reg |= DIAG_ALATCH;
    if (invertPolarity)
        reg |= DIAG_APOL;
    if (convReadyEnable)
        reg |= DIAG_CNVR;
    if (slowAlert)
        reg |= DIAG_SLOWALERT;
    writeReg(0x0F, reg);
}

bool INA237::isConversionReady() { return (readReg(0x0F) & DIAG_CNVRF) != 0; }

uint16_t INA237::readManufacturerId() { return readReg(INA237Reg::MANUFACTURER_ID); }

uint16_t INA237::readDeviceId() { return readReg(INA237Reg::DEVICE_ID); }

// ── Privé ────────────────────────────────────────────────────────────────────

void INA237::calibrate(float shuntOhms, float maxCurrentA) {
    // currentLSB = MaxExpectedCurrent / 2^15
    _currentLSB = maxCurrentA / 32768.0f;

    // SHUNT_CAL = 13107.2e6 × CurrentLSB × Rshunt
    //   (facteur de la datasheet p.21 : 13107.2 × 10^6 pour plage ±163.84mV)
    //   Pour plage ±40.96mV le facteur est ×4 : 52428.8 × 10^6
    float factor = (_range == ADCRange::RANGE_163_84mV) ? 13107.2e6f : 52428.8e6f;
    uint16_t shuntCal = static_cast<uint16_t>(factor * _currentLSB * shuntOhms);
    writeReg(INA237Reg::SHUNT_CAL, shuntCal);
}

uint16_t INA237::readReg(uint8_t reg) {
    _wire.beginTransmission(_addr);
    _wire.write(reg);
    if (_wire.endTransmission(false) != 0)
        return 0xFFFF;
    _wire.requestFrom(static_cast<uint8_t>(_addr), static_cast<uint8_t>(2));
    if (_wire.available() < 2)
        return 0xFFFF;
    uint16_t val = (static_cast<uint16_t>(_wire.read()) << 8);
    val |= static_cast<uint16_t>(_wire.read());
    return val;
}

void INA237::writeReg(uint8_t reg, uint16_t value) {
    _wire.beginTransmission(_addr);
    _wire.write(reg);
    _wire.write(static_cast<uint8_t>((value >> 8) & 0xFF));
    _wire.write(static_cast<uint8_t>(value & 0xFF));
    _wire.endTransmission();
}
