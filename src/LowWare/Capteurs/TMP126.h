#pragma once
/**
 * @file TMP126.h
 * @brief Driver C++ pour le Texas Instruments TMP126
 *        Capteur de température SPI 3-wire, 14-bit, ±0.25°C, -55°C à 175°C
 *
 * Datasheet : SNIS209C – TI TMP126
 *
 * Interface SPI :
 *   - Mode 0 (CPOL=0, CPHA=0) ou Mode 1 (CPOL=0, CPHA=1) selon câblage
 *     → La datasheet précise : données clocked OUT sur falling edge SCLK,
 *       clocked IN sur rising edge → SPI Mode 0 (CPOL=0, CPHA=0) côté ESP32
 *   - 3-wire : CS, SCLK, SIO (half-duplex bidirectionnel)
 *     → Sur ESP32, utiliser MOSI+MISO reliés ensemble via résistance 10kΩ,
 *       ou utiliser le bus SPI full-duplex avec MOSI/MISO séparés si possible.
 *   - Fréquence max : 10 MHz
 *   - Transferts de 16 bits (command word + data word)
 *
 * Structure du Command Word (16 bits) :
 *   Bit 15     : Don't care
 *   Bit 14     : CRC Enable
 *   Bits[13:10]: CRC Data Block Length
 *   Bit 9      : Auto Increment
 *   Bit 8      : R/W (1=Read, 0=Write)
 *   Bits[7:0]  : Sub-Address (registre)
 *
 * Registres couverts :
 *   TEMP_RESULT, ALERT_STATUS, CONFIG, THIGH_LIMIT, TLOW_LIMIT,
 *   HYSTERESIS, SLEW_LIMIT, ALERT_ENABLE, DEVICE_ID
 */

#include "Config/config.h"
#include <Arduino.h>
#include <SPI.h>
#include <cmath>
#include <cstdint>


// ─────────────────────────────────────────────
//  Adresses des registres
// ─────────────────────────────────────────────
namespace TMP126Reg {
    constexpr uint8_t TEMP_RESULT = 0x00; ///< Résultat température (RO)
    constexpr uint8_t ALERT_STATUS = 0x01; ///< Statuts alertes (R/clear)
    constexpr uint8_t CONFIG = 0x03; ///< Configuration
    constexpr uint8_t THIGH_LIMIT = 0x04; ///< Limite haute température
    constexpr uint8_t TLOW_LIMIT = 0x05; ///< Limite basse température
    constexpr uint8_t HYSTERESIS = 0x06; ///< Hystérésis
    constexpr uint8_t SLEW_LIMIT = 0x07; ///< Limite slew rate
    constexpr uint8_t ALERT_ENABLE = 0x08; ///< Activation des alertes
    constexpr uint8_t DEVICE_ID = 0x0F; ///< ID device (RO)
} // namespace TMP126Reg

// ─────────────────────────────────────────────
//  Bits du registre CONFIG
// ─────────────────────────────────────────────
namespace TMP126Config {
    // Mode opératoire
    constexpr uint16_t MODE_CONTINUOUS = 0u << 10; ///< Mode continu (défaut)
    constexpr uint16_t MODE_SHUTDOWN = 1u << 10; ///< Shutdown

    // One-shot (en mode shutdown)
    constexpr uint16_t ONE_SHOT = 1u << 9;

    // Période de conversion (Conv_Period[2:0] bits[8:6])
    constexpr uint16_t CONV_31MS = 0u << 6; ///< 31.25 ms  (~32 Hz)
    constexpr uint16_t CONV_62MS = 1u << 6; ///< 62.5  ms  (~16 Hz)
    constexpr uint16_t CONV_125MS = 2u << 6; ///< 125   ms  (~8  Hz)
    constexpr uint16_t CONV_250MS = 3u << 6; ///< 250   ms  (~4  Hz)
    constexpr uint16_t CONV_500MS = 4u << 6; ///< 500   ms  (~2  Hz)
    constexpr uint16_t CONV_1S = 5u << 6; ///< 1     s   (~1  Hz)
    constexpr uint16_t CONV_4S = 6u << 6; ///< 4     s
    constexpr uint16_t CONV_16S = 7u << 6; ///< 16    s

    // Averaging (AVG[1:0] bits[5:4])
    constexpr uint16_t AVG_1 = 0u << 4; ///< Pas de moyennage
    constexpr uint16_t AVG_8 = 1u << 4; ///< 8 mesures moyennées
    constexpr uint16_t AVG_32 = 2u << 4; ///< 32 mesures
    constexpr uint16_t AVG_64 = 3u << 4; ///< 64 mesures

    // Mode alerte
    constexpr uint16_t INT_MODE = 0u << 3; ///< Interrupt mode
    constexpr uint16_t COMP_MODE = 1u << 3; ///< Comparator mode

    // Activation Data_Ready sur ALERT
    constexpr uint16_t DATA_READY_EN = 1u << 2;

    // Polarité ALERT (0=active-low, 1=active-high)
    constexpr uint16_t ALERT_POL_HIGH = 1u << 1;

    // Software reset
    constexpr uint16_t SOFT_RESET = 1u << 0;
} // namespace TMP126Config

// ─────────────────────────────────────────────
//  Bits du registre ALERT_STATUS
// ─────────────────────────────────────────────
namespace TMP126Alert {
    constexpr uint16_t CRC_FLAG = 1u << 8; ///< Erreur CRC
    constexpr uint16_t SLEW_FLAG = 1u << 5; ///< Slew rate dépassé
    constexpr uint16_t SLEW_STATUS = 1u << 4; ///< Statut slew rate
    constexpr uint16_t THIGH_FLAG = 1u << 3; ///< Flag limite haute
    constexpr uint16_t THIGH_STATUS = 1u << 2; ///< Statut limite haute
    constexpr uint16_t TLOW_FLAG = 1u << 1; ///< Flag limite basse
    constexpr uint16_t TLOW_STATUS = 1u << 0; ///< Statut limite basse
    constexpr uint16_t DATA_READY = 1u << 9; ///< Donnée prête
} // namespace TMP126Alert

// ─────────────────────────────────────────────
//  Bits du registre ALERT_ENABLE
// ─────────────────────────────────────────────
namespace TMP126AlertEn {
    constexpr uint16_t SLEW_EN = 1u << 2;
    constexpr uint16_t THIGH_EN = 1u << 1;
    constexpr uint16_t TLOW_EN = 1u << 0;
} // namespace TMP126AlertEn

// ─────────────────────────────────────────────
//  Structure d'état alertes parsée
// ─────────────────────────────────────────────
struct TMP126AlertStatus {
    bool dataReady; ///< Conversion terminée
    bool crcError; ///< Erreur CRC détectée par le TMP126
    bool slewFlag; ///< Slew rate flag (interrupt mode: front)
    bool slewStatus; ///< Slew rate status (comparator mode: niveau)
    bool thighFlag;
    bool thighStatus; ///< Température > THighLimit
    bool tlowFlag;
    bool tlowStatus; ///< Température < TLowLimit
};

// ─────────────────────────────────────────────
//  Constante physique
// ─────────────────────────────────────────────
static constexpr float TMP126_LSB_DEG = 0.03125f; ///< 1 LSB = 0.03125 °C
static constexpr uint16_t TMP126_DEVICE_ID_EXPECTED = 0x1126; ///< ID attendu

// ─────────────────────────────────────────────
//  Classe principale
// ─────────────────────────────────────────────
class TMP126 {
public:
    /**
     * @brief Constructeur
     * @param csPin   Pin Chip Select (actif bas)
     * @param spi     Bus SPI (défaut : SPI)
     * @param spiFreq Fréquence SPI en Hz (max 10 MHz)
     */
    explicit TMP126();
    ~TMP126();

    /**
     * @brief Initialise le SPI et vérifie l'ID du device
     * @return true si le device répond correctement
     */
    bool init() const;

    // ── Reset ──────────────────────────────
    /** Reset logiciel (bit SOFT_RESET dans CONFIG) */
    void softReset() const;

    // ── Configuration ──────────────────────
    /**
     * @brief Configure le device en mode continu
     * @param convPeriod  Période de conversion (TMP126Config::CONV_xxx)
     * @param averaging   Moyennage (TMP126Config::AVG_xxx)
     */
    void setContinuousMode(uint16_t convPeriod = TMP126Config::CONV_1S, uint16_t averaging = TMP126Config::AVG_1) const;

    /** Configure le device en mode shutdown (basse consommation ~350 nA) */
    void setShutdownMode() const;

    /**
     * @brief Déclenche une conversion one-shot (depuis shutdown)
     *        La conversion dure ~6 ms, puis le device revient en shutdown.
     */
    void triggerOneShot() const;

    /**
     * @brief Configure le mode de l'alerte
     * @param comparatorMode true=Comparator, false=Interrupt
     * @param alertActiveHigh true=ALERT actif haut, false=actif bas (défaut)
     * @param dataReadyOnAlert true=active Data_Ready sur pin ALERT
     */
    void configureAlert(bool comparatorMode = false, bool alertActiveHigh = false, bool dataReadyOnAlert = false) const;

    // ── Limites d'alerte ───────────────────
    /** Limite haute température en °C */
    void setHighLimit(float tempCelsius) const;

    /** Limite basse température en °C */
    void setLowLimit(float tempCelsius) const;

    /**
     * @brief Hystérésis (MSB = THigh_Hyst, LSB = TLow_Hyst)
     * @param thighHystCelsius Hystérésis limite haute (°C)
     * @param tlowHystCelsius  Hystérésis limite basse (°C)
     */
    void setHysteresis(float thighHystCelsius, float tlowHystCelsius = 0.0f) const;

    /**
     * @brief Limite de slew rate (variation positive de T entre 2 conversions)
     * @param slewLimitCelsius Limite en °C par période de conversion
     */
    void setSlewLimit(float slewLimitCelsius) const;

    /**
     * @brief Active / désactive les sources d'alerte sur pin ALERT
     * @param thigh  Alerte limite haute
     * @param tlow   Alerte limite basse
     * @param slew   Alerte slew rate
     */
    void enableAlerts(bool thigh, bool tlow, bool slew = false) const;

    // ── Lectures ───────────────────────────
    /**
     * @brief Lit la température en °C
     * @return Température en °C, NAN si erreur de communication
     */
    float readTemperature() const;

    /**
     * @brief Lit et parse le registre ALERT_STATUS
     *        La lecture efface les flag bits en mode interrupt.
     */
    TMP126AlertStatus readAlertStatus() const;

    /** Vrai si la conversion est prête (Data_Ready dans ALERT_STATUS) */
    bool isDataReady() const;

    // ── Identification ─────────────────────
    uint16_t readDeviceId() const;

    // ── Accès bas niveau ───────────────────
    uint16_t readReg(uint8_t reg) const;
    void writeReg(uint8_t reg, uint16_t value) const;

    // ── One-shot bloquant ──────────────────
    /**
     * @brief Effectue une mesure one-shot bloquante
     * @param timeoutMs Timeout en ms (défaut 20 ms, conversion ~6 ms typ.)
     * @return Température en °C, NAN si timeout
     */
    float readOneShotBlocking(uint32_t timeoutMs = 20) const;

private:
    SPIClass *_spi;
    uint8_t _csPin;
    uint32_t _spiFreq;

    SPISettings _spiSettings;

    /**
     * Envoie un command word et retourne le mot de données reçu.
     * Pour une lecture, le data word est reçu pendant le 2e transfert.
     * Pour une écriture, le data word est envoyé pendant le 2e transfert.
     */
    uint16_t _transfer16(uint16_t txWord) const;
    static uint16_t _buildCmd(uint8_t reg, bool read, bool autoInc = false);
    int16_t _rawToSigned(uint16_t raw);
    static uint16_t _tempToRaw(float tempCelsius);
    static float _rawToTemp(uint16_t raw);

    void _csLow() const;
    void _csHigh() const;
};
