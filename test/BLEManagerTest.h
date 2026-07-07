#pragma once
#include <string>
#include "../src/Interface/BLEInterface.h"

class BLEManagerTest : public BLEInterface {
public:
    float lastCurrent = 0.0f;
    float lastPcbTemp = 0.0f;
    uint32_t lastAlarmStatus = 0;
    std::string lastLogMessage = "";

    bool mockIsConnected = false;
    AppConfig mockConfig = {LimitConfig{}, false};

    void init(const std::string &deviceName) override {}

    void updateTelemetry(float current, float pcbTemp, float ambTemp1, float ambTemp2, uint32_t alarmStatus) override {
        lastCurrent = current;
        lastPcbTemp = pcbTemp;
        lastAlarmStatus = alarmStatus;
    }

    void updateLogs(const std::string &logMessage) override { lastLogMessage = logMessage; }

    AppConfig getConfig() const override { return mockConfig; }

    void clearUpdateFlag() override { mockConfig.isUpdated = false; }

    bool isConnected() const override { return mockIsConnected; }

    void simulateClientWriteConfig(float maxCurrent, float maxTempPcb) {
        mockConfig.config.current_limit_high = maxCurrent;
        mockConfig.config.temp_pcb_limit_high = maxTempPcb;
        mockConfig.isUpdated = true;
    }
};
