#include "INA237.h"

static constexpr uint16_t MANUFACTURER_ID_EXPECTED = 0x5449; // 'TI'
static constexpr uint16_t CONFIG_RST_BIT = 1u << 15;
static constexpr uint16_t CONFIG_ADCRANGE_BIT = 1u << 4;
static constexpr uint16_t DIAG_CNVRF = 1u << 1;

INA237::INA237(INA237Transport &transport, float shuntOhms, float maxCurrentA) :
    _transport(transport), _shuntOhms(shuntOhms), _maxCurrentA(maxCurrentA) {}

bool INA237::init(const ADCRange range) {
    _range = range;
    if (!_transport.initBus()) {
        return false;
    }

    if (readManufacturerId() != MANUFACTURER_ID_EXPECTED) {
        return false;
    }

    reset();
    _transport.delayMs(2);

    uint16_t cfg = 0;
    if (range == ADCRange::RANGE_40_96mV)
        cfg |= CONFIG_ADCRANGE_BIT;
    _transport.writeReg(INA237Reg::CONFIG, cfg);

    constexpr uint16_t adcCfg = static_cast<uint16_t>(OperatingMode::CONT_ALL) << 12 |
                                static_cast<uint16_t>(ConvTime::US_1052) << 9 |
                                static_cast<uint16_t>(ConvTime::US_1052) << 6 |
                                static_cast<uint16_t>(ConvTime::US_1052) << 3 | static_cast<uint16_t>(Averaging::AVG_1);
    _transport.writeReg(INA237Reg::ADC_CONFIG, adcCfg);

    calibrate(_shuntOhms, _maxCurrentA);
    return true;
}

void INA237::reset() const { _transport.writeReg(INA237Reg::CONFIG, CONFIG_RST_BIT); }

void INA237::setMode(OperatingMode mode) const {
    uint16_t reg = _transport.readReg(INA237Reg::ADC_CONFIG);
    reg = (reg & 0x0FFF) | (static_cast<uint16_t>(mode) << 12);
    _transport.writeReg(INA237Reg::ADC_CONFIG, reg);
}

float INA237::readShuntVoltage() const {
    const auto raw = static_cast<int16_t>(_transport.readReg(INA237Reg::VSHUNT));
    return static_cast<float>(raw) * shuntLSB_uV() * 1e-6f;
}

float INA237::readBusVoltage() const {
    const auto raw = static_cast<uint16_t>(_transport.readReg(INA237Reg::VBUS));
    return static_cast<float>(raw) * 3.125e-3f; // LSB = 3.125 mV
}

float INA237::readTemperature() const {
    auto raw = static_cast<int16_t>(_transport.readReg(INA237Reg::DIETEMP));
    raw >>= 4;
    return static_cast<float>(raw) * 0.125f;
}

float INA237::readCurrent() const {
    const auto raw = static_cast<int16_t>(_transport.readReg(INA237Reg::CURRENT));
    return static_cast<float>(raw) * _currentLSB;
}

float INA237::readPower() const {
    const uint16_t raw = _transport.readReg(INA237Reg::POWER);
    return static_cast<float>(raw) * 3.2f * _currentLSB;
}

uint16_t INA237::readManufacturerId() const { return _transport.readReg(INA237Reg::MANUFACTURER_ID); }

bool INA237::isConversionReady() const { return (_transport.readReg(0x0F) & DIAG_CNVRF) != 0; }

void INA237::calibrate(const float shuntOhms, const float maxCurrentA) {
    _currentLSB = (maxCurrentA / 32768.0f * 100);
    uint16_t shuntCal = calcShuntCal(_currentLSB, shuntOhms, _range);
    _transport.writeReg(INA237Reg::SHUNT_CAL, shuntCal);
}

// Fonction statique extraite pour pouvoir être testée unitairement facilement
uint16_t INA237::calcShuntCal(float currentLSB, float shuntOhms, ADCRange range) {
    const float factor = range == ADCRange::RANGE_163_84mV ? 13107.2e6f : 52428.8e6f;
    return static_cast<uint16_t>(factor * currentLSB * shuntOhms);
}
