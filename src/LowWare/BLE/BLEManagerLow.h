#pragma once
#include <BLE2902.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

#include "Interface/BLEInterface.h"

class BLEManagerLow : public BLEInterface, public BLEServerCallbacks, public BLECharacteristicCallbacks {
public:
    BLEManagerLow();
    ~BLEManagerLow() override = default;

    void init(const std::string &deviceName) override;
    void updateTelemetry(float current, float pcbTemp, float ambTemp1, float ambTemp2, uint32_t alarmStatus) override;
    void updateLogs(const std::string &logMessage) override;
    AppConfig getConfig() const override;
    void clearUpdateFlag() override;
    bool isConnected() const override;

protected:
    void onConnect(BLEServer *pServer) override;
    void onDisconnect(BLEServer *pServer) override;

    void onWrite(BLECharacteristic *pCharacteristic) override;

private:
    BLEServer *_server;

    BLECharacteristic *_charCurrent;
    BLECharacteristic *_charPcbTemp;
    BLECharacteristic *_charAmbTemp1;
    BLECharacteristic *_charAmbTemp2;
    BLECharacteristic *_charAlarm;
    BLECharacteristic *_charLogs;

    BLECharacteristic *_charCfgMaxCurrent;
    BLECharacteristic *_charCfgMaxTempPcb;
    BLECharacteristic *_charCfgMaxTempAmb;

    bool _isConnected;
    AppConfig _currentConfig;

    void setFloatValue(BLECharacteristic *pChar, float value);
};
