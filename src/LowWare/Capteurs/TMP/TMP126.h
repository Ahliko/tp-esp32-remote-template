#pragma once
#include <cmath>
#include <cstdint>

#include "Interface/TMP126Transport.h"

namespace TMP126Reg {
    constexpr uint8_t TEMP_RESULT  = 0x00; ///< Résultat température (RO)
    constexpr uint8_t ALERT_STATUS = 0x01; ///< Statuts alertes (R/clear)
    constexpr uint8_t CONFIG       = 0x03; ///< Configuration
    constexpr uint8_t THIGH_LIMIT  = 0x04; ///< Limite haute température
    constexpr uint8_t TLOW_LIMIT   = 0x05; ///< Limite basse température
    constexpr uint8_t HYSTERESIS   = 0x06; ///< Hystérésis
    constexpr uint8_t SLEW_LIMIT   = 0x07; ///< Limite slew rate
    constexpr uint8_t ALERT_ENABLE = 0x08; ///< Activation des alertes
    constexpr uint8_t DEVICE_ID    = 0x0C; ///< ID device (RO)
}

namespace TMP126Config {
    constexpr uint16_t MODE_CONTINUOUS = 0u << 10;
    constexpr uint16_t MODE_SHUTDOWN   = 1u << 10;
    constexpr uint16_t ONE_SHOT        = 1u << 9;
    constexpr uint16_t CONV_31MS       = 0u << 6;
    constexpr uint16_t CONV_62MS       = 1u << 6;
    constexpr uint16_t CONV_125MS      = 2u << 6;
    constexpr uint16_t CONV_250MS      = 3u << 6;
    constexpr uint16_t CONV_500MS      = 4u << 6;
    constexpr uint16_t CONV_1S         = 5u << 6;
    constexpr uint16_t CONV_4S         = 6u << 6;
    constexpr uint16_t CONV_16S        = 7u << 6;
    constexpr uint16_t AVG_1           = 0u << 4;
    constexpr uint16_t AVG_8           = 1u << 4;
    constexpr uint16_t AVG_32          = 2u << 4;
    constexpr uint16_t AVG_64          = 3u << 4;
    constexpr uint16_t INT_MODE        = 0u << 3;
    constexpr uint16_t COMP_MODE       = 1u << 3;
    constexpr uint16_t DATA_READY_EN   = 1u << 2;
    constexpr uint16_t ALERT_POL_HIGH  = 1u << 1;
    constexpr uint16_t SOFT_RESET      = 1u << 0;
}

namespace TMP126Alert {
    constexpr uint16_t CRC_FLAG     = 1u << 8;
    constexpr uint16_t SLEW_FLAG    = 1u << 5;
    constexpr uint16_t SLEW_STATUS  = 1u << 4;
    constexpr uint16_t THIGH_FLAG   = 1u << 3;
    constexpr uint16_t THIGH_STATUS = 1u << 2;
    constexpr uint16_t TLOW_FLAG    = 1u << 1;
    constexpr uint16_t TLOW_STATUS  = 1u << 0;
    constexpr uint16_t DATA_READY   = 1u << 9;
}

namespace TMP126AlertEn {
    constexpr uint16_t SLEW_EN  = 1u << 2;
    constexpr uint16_t THIGH_EN = 1u << 1;
    constexpr uint16_t TLOW_EN  = 1u << 0;
}

struct TMP126AlertStatus {
    bool dataReady;
    bool crcError;
    bool slewFlag;
    bool slewStatus;
    bool thighFlag;
    bool thighStatus;
    bool tlowFlag;
    bool tlowStatus;
};

static constexpr float TMP126_LSB_DEG = 0.03125f;
static constexpr uint16_t TMP126_DEVICE_ID_EXPECTED = 0x1126;

class TMP126 {
public:
    explicit TMP126(TMP126Transport& transport);
    ~TMP126() = default;

    bool init() const;
    void softReset() const;

    void setContinuousMode(uint16_t convPeriod = TMP126Config::CONV_1S, uint16_t averaging = TMP126Config::AVG_1) const;
    void setShutdownMode() const;
    void triggerOneShot() const;
    void configureAlert(bool comparatorMode = false, bool alertActiveHigh = false, bool dataReadyOnAlert = false) const;

    void setHighLimit(float tempCelsius) const;
    void setLowLimit(float tempCelsius) const;
    void setHysteresis(float thighHystCelsius, float tlowHystCelsius = 0.0f) const;
    void setSlewLimit(float slewLimitCelsius) const;
    void enableAlerts(bool thigh, bool tlow, bool slew = false) const;

    float readTemperature() const;
    TMP126AlertStatus readAlertStatus() const;
    bool isDataReady() const;
    uint16_t readDeviceId() const;
    float readOneShotBlocking(uint32_t timeoutMs = 20) const;

    // Conversions pures (statiques)
    static uint16_t buildCmd(uint8_t reg, bool read, bool autoInc = false);
    static float rawToTemp(uint16_t raw);
    static uint16_t tempToRaw(float tempCelsius);
    static TMP126AlertStatus parseAlertStatus(uint16_t regValue);

private:
    TMP126Transport& _transport;

    uint16_t readReg(uint8_t reg) const;
    void writeReg(uint8_t reg, uint16_t value) const;
};