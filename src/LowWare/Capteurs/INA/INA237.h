#pragma once
#include <cstdint>

#include "Interface/INA237Transport.h"

namespace INA237Reg {
    constexpr uint8_t CONFIG = 0x00;
    constexpr uint8_t ADC_CONFIG = 0x01;
    constexpr uint8_t SHUNT_CAL = 0x02;
    constexpr uint8_t SHUNT_TEMPCO = 0x03;
    constexpr uint8_t VSHUNT = 0x04;
    constexpr uint8_t VBUS = 0x05;
    constexpr uint8_t DIETEMP = 0x06;
    constexpr uint8_t CURRENT = 0x07;
    constexpr uint8_t POWER = 0x08;
    constexpr uint8_t MANUFACTURER_ID = 0x3E;
} // namespace INA237Reg

enum class ADCRange : uint8_t { RANGE_163_84mV = 0, RANGE_40_96mV = 1 };
enum class ConvTime : uint8_t {
    US_50 = 0,
    US_84 = 1,
    US_150 = 2,
    US_280 = 3,
    US_540 = 4,
    US_1052 = 5,
    US_2074 = 6,
    US_4120 = 7
};
enum class Averaging : uint8_t {
    AVG_1 = 0,
    AVG_4 = 1,
    AVG_16 = 2,
    AVG_64 = 3,
    AVG_128 = 4,
    AVG_256 = 5,
    AVG_512 = 6,
    AVG_1024 = 7
};
enum class OperatingMode : uint8_t { SHUTDOWN = 0x0, CONT_ALL = 0xF };

class INA237 {
public:
    explicit INA237(INA237Transport &transport, float shuntOhms, float maxCurrentA);
    ~INA237() = default;

    bool init(ADCRange range = ADCRange::RANGE_163_84mV);
    void reset() const;

    void setMode(OperatingMode mode) const;
    void setShuntConvTime(ConvTime ct) const;
    void setBusConvTime(ConvTime ct) const;
    void setTempConvTime(ConvTime ct) const;
    void setAveraging(Averaging avg) const;
    void setConversionDelay(uint8_t delay2ms) const;
    void setShuntTempCoeff(uint16_t ppmPerCelsius) const;

    float readShuntVoltage() const;
    float readBusVoltage() const;
    float readTemperature() const;
    float readCurrent() const;
    float readPower() const;

    uint16_t readManufacturerId() const;
    bool isConversionReady() const;

    float getCurrentLSB() const { return _currentLSB; }

    static uint16_t calcShuntCal(float currentLSB, float shuntOhms, ADCRange range);

private:
    INA237Transport &_transport;
    float _shuntOhms;
    float _maxCurrentA;
    float _currentLSB{0.0f};
    ADCRange _range{ADCRange::RANGE_163_84mV};

    float shuntLSB_uV() const { return _range == ADCRange::RANGE_163_84mV ? 5.0f : 1.25f; }

    void calibrate(float shuntOhms, float maxCurrentA);
};
