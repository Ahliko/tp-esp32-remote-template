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
    // Atténuation à 11dB pour permettre des lectures jusqu'à ~3.1V sur l'ESP32
    analogSetPinAttenuation(_pin, ADC_11db);
}

float Thermistor::readTemperature(const uint8_t samples) const {
    uint32_t totalMv = 0;

    // Sur-échantillonnage pour lisser le bruit de l'ADC de l'ESP32
    for (uint8_t i = 0; i < samples; i++) {
        // analogReadMilliVolts utilise la calibration interne eFuse de l'ESP32
        totalMv += analogReadMilliVolts(_pin);
        delay(1);
    }
    
    const float averageMv = static_cast<float>(totalMv) / samples;

    // Sécurité pour éviter les divisions par zéro
    if (averageMv <= 0.0f || averageMv >= VCC_MV) {
        return NAN; // Circuit ouvert ou court-circuit
    }

    // Calcul de la résistance de la thermistance selon le câblage
    float thermistorResistance;
    if (_thermistorToGround) {
        // Câblage : VCC -> Résistance série -> ADC -> Thermistance -> GND
        thermistorResistance = _seriesResistor / ((VCC_MV / averageMv) - 1.0f);
    } else {
        // Câblage : VCC -> Thermistance -> ADC -> Résistance série -> GND
        thermistorResistance = _seriesResistor * ((VCC_MV / averageMv) - 1.0f);
    }

    // Équation de Steinhart-Hart simplifiée (Beta)
    // T = 1 / (1/To + (1/Beta) * ln(R/Ro))
    float steinhart = thermistorResistance / _nominalResistor; // (R/Ro)
    steinhart = log(steinhart);                                // ln(R/Ro)
    steinhart /= _betaValue;                                   // 1/B * ln(R/Ro)
    steinhart += 1.0f / NOMINAL_TEMPERATURE_K;                 // + (1/To)
    steinhart = 1.0f / steinhart;                              // Inversion

    return steinhart + ABSOLUTE_ZERO_C;                        // Conversion Kelvin vers Celsius
}