#pragma once
/**
 * @file INA237.h
 * @brief Driver C++ pour le Texas Instruments INA237 - Power Monitor 85V/16-bit
 *        Compatible ESP32 (Arduino framework & ESP-IDF via arduino-esp32)
 *
 * Datasheet: SBOSA20A - TI INA237
 * Registres couverts : CONFIG, ADC_CONFIG, SHUNT_CAL, SHUNT_TEMPCO,
 *                      VSHUNT, VBUS, DIETEMP, CURRENT, POWER,
 *                      SOVL, SUVL, BOVL, BUVL, TEMP_LIMIT, PWR_LIMIT,
 *                      DIAG_ALRT, MANUFACTURER_ID, DEVICE_ID
 */

#include <Wire.h>
#include <cstdint>



// ─────────────────────────────────────────────
//  Registres
// ─────────────────────────────────────────────
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
    constexpr uint8_t SOVL = 0x09; // Shunt Over-Voltage Limit
    constexpr uint8_t SUVL = 0x0A; // Shunt Under-Voltage Limit
    constexpr uint8_t BOVL = 0x0B; // Bus Over-Voltage Limit
    constexpr uint8_t BUVL = 0x0C; // Bus Under-Voltage Limit
    constexpr uint8_t TEMP_LIMIT = 0x0D;
    constexpr uint8_t PWR_LIMIT = 0x0E;
    constexpr uint8_t DIAG_ALRT = 0x0B + 4; // 0x0F
    constexpr uint8_t SOVL_REG = 0x09;
    constexpr uint8_t MANUFACTURER_ID = 0x3E;
} // namespace INA237Reg

// ─────────────────────────────────────────────
//  Énumérations de configuration
// ─────────────────────────────────────────────

/** Plage shunt : ADCRANGE bit dans CONFIG */
enum class ADCRange : uint8_t {
    RANGE_163_84mV = 0, ///< ±163.84 mV, LSB = 5 µV
    RANGE_40_96mV = 1, ///< ±40.96  mV, LSB = 1.25 µV
};

/** Délai de conversion (CONVDLY) : 0..255 → 0..510 ms (pas de 2 ms) */

/** Temps de conversion ADC */
enum class ConvTime : uint8_t {
    US_50 = 0,
    US_84 = 1,
    US_150 = 2,
    US_280 = 3,
    US_540 = 4,
    US_1052 = 5,
    US_2074 = 6,
    US_4120 = 7,
};

/** Nombre de moyennes */
enum class Averaging : uint8_t {
    AVG_1 = 0,
    AVG_4 = 1,
    AVG_16 = 2,
    AVG_64 = 3,
    AVG_128 = 4,
    AVG_256 = 5,
    AVG_512 = 6,
    AVG_1024 = 7,
};

enum class OperatingMode : uint8_t {
    SHUTDOWN = 0x0,
    TRIG_BUS = 0x1,
    TRIG_SHUNT = 0x2,
    TRIG_SHUNT_BUS = 0x3,
    TRIG_TEMP = 0x4,
    TRIG_TEMP_BUS = 0x5,
    TRIG_TEMP_SHUNT = 0x6,
    TRIG_ALL = 0x7,
    CONT_BUS = 0x9,
    CONT_SHUNT = 0xA,
    CONT_SHUNT_BUS = 0xB,
    CONT_TEMP = 0xC,
    CONT_TEMP_BUS = 0xD,
    CONT_TEMP_SHUNT = 0xE,
    CONT_ALL = 0xF,
};


class INA237 {
public:
    /**
     * @brief Constructeur
     * @param address Adresse I2C (défaut : A1=GND, A0=GND → 0x40)
     * @param wire    Bus I2C (défaut : Wire)
     */
    explicit INA237();
    ~INA237();

    /**
     * @brief Initialisation. Doit être appelé après Wire.begin().
     * @param shuntOhms   Résistance de shunt en Ohms
     * @param maxCurrentA Courant maximum attendu en Ampères
     * @param range       Plage ADC shunt
     * @return true si le device répond et l'ID est valide
     */
    bool init(ADCRange range = ADCRange::RANGE_163_84mV);

    /** Reset logiciel (bit RST dans CONFIG) */
    void reset() const;

    // ── Configuration ADC ─────────────────────
    void setMode(OperatingMode mode) const;
    void setShuntConvTime(ConvTime ct) const;
    void setBusConvTime(ConvTime ct) const;
    void setTempConvTime(ConvTime ct) const;
    void setAveraging(Averaging avg) const;

    /** Délai de conversion : 0–255 (multiplié par 2 ms) */
    void setConversionDelay(uint8_t delay2ms) const;

    /** Active/désactive la correction de température du shunt (TEMPCO) */
    void setShuntTempCoeff(uint16_t ppmPerCelsius) const;

    // ── Lectures ─────────────────────────────
    /** Tension shunt en Volts */
    float readShuntVoltage() const;

    /** Tension bus en Volts */
    float readBusVoltage() const;

    /** Température die en °C */
    float readTemperature() const;

    /** Courant en Ampères (nécessite calibration SHUNT_CAL) */
    float readCurrent() const;

    /** Puissance en Watts */
    float readPower() const;


    // ── Identifiants ─────────────────────────
    uint16_t readManufacturerId() const; ///< Doit retourner 0x5449 ('TI')

    /** Vrai si une conversion est prête (polling CNVRF) */
    bool isConversionReady() const;

    /** Courant LSB calculé après begin() [A/bit] */
    float getCurrentLSB() const { return _currentLSB; }

private:
    TwoWire *_wire;
    float _currentLSB{0.0f};
    float _shuntOhms{0.0f};
    ADCRange _range{ADCRange::RANGE_163_84mV};

    // LSB shunt selon plage (µV)
    float shuntLSB_uV() const { return _range == ADCRange::RANGE_163_84mV ? 5.0f : 1.25f; }

    uint16_t readReg(uint8_t reg) const;
    void writeReg(uint8_t reg, uint16_t value) const;

    /** Calcule et écrit SHUNT_CAL */
    void calibrate(float shuntOhms, float maxCurrentA);
};
