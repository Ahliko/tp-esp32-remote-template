#pragma once

#include <Arduino.h>

class Thermistor {
public:
    /**
     * @brief Constructeur pour une thermistance NTC
     *
     * @param pin Broche ADC de l'ESP32
     * @param seriesResistor Résistance du pont diviseur (ex: 10000.0 pour 10k)
     * @param nominalResistor Résistance de la thermistance à 25°C (ex: 10000.0)
     * @param betaValue Valeur Beta de la thermistance (ex: 3950.0)
     * @param thermistorToGround True si la thermistance est reliée au GND, False si elle est reliée au VCC
     */
    explicit Thermistor(uint8_t pin, float seriesResistor = 10000.0f, float nominalResistor = 10000.0f,
                        float betaValue = 3950.0f, bool thermistorToGround = true);

    ~Thermistor() = default;

    /**
     * @brief Initialise la broche ADC
     */
    void init() const;

    /**
     * @brief Lit et calcule la température en degrés Celsius
     * @param samples Nombre d'échantillons pour le sur-échantillonnage (lissage)
     * @return Température en °C
     */
    float readTemperature(uint8_t samples = 10) const;

private:
    uint8_t _pin;
    float _seriesResistor;
    float _nominalResistor;
    float _betaValue;
    bool _thermistorToGround;

    static constexpr float NOMINAL_TEMPERATURE_K = 298.15f; // 25°C en Kelvin
    static constexpr float ABSOLUTE_ZERO_C = -273.15f;
    static constexpr float VCC_MV = 3300.0f; // Tension de référence de l'ESP32 (~3.3V) en mV
};
