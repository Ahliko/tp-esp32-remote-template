#include "Thermistor.h"

Thermistor::Thermistor(const uint8_t pin, const float seriesResistor, 
                       const float nominalResistor, const float betaValue, 
                       const bool thermistorToGround)
    : _pin(pin), 
      _seriesResistor(seriesResistor), 
      _nominalResistor(nominalResistor), 
      _betaValue(betaValue),
      _thermistorToGround(thermistorToGround) 
{
}

void Thermistor::init() const {
    pinMode(_pin, INPUT);
    analogSetPinAttenuation(_pin, ADC_11db);
}

float Thermistor::readTemperature(const uint8_t samples) const {
    uint32_t totalMv = 0;

    for (uint8_t i = 0; i < samples; i++) {
        totalMv += analogReadMilliVolts(_pin);
        delay(1);
    }

    const float averageMv = static_cast<float>(totalMv) / samples;

    if (averageMv <= 0.0f || averageMv >= VCC_MV) {
        return NAN;
    }

    float thermistorResistance;
    if (_thermistorToGround) {
        thermistorResistance = _seriesResistor / ((VCC_MV / averageMv) - 1.0f);
    } else {
        thermistorResistance = _seriesResistor * ((VCC_MV / averageMv) - 1.0f);
    }

    float steinhart = thermistorResistance / _nominalResistor;
    steinhart = log(steinhart);
    steinhart /= _betaValue;
    steinhart += 1.0f / NOMINAL_TEMPERATURE_K;
    steinhart = 1.0f / steinhart;

    return steinhart + ABSOLUTE_ZERO_C;
}