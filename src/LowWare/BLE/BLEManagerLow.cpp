#include "BLEManagerLow.h"
#include <Arduino.h>

// UUIDs
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHAR_CURRENT_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define CHAR_PCB_TEMP_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a9"
#define CHAR_AMB1_TEMP_UUID "beb5483e-36e1-4688-b7f5-ea07361b26aa"
#define CHAR_AMB2_TEMP_UUID "beb5483e-36e1-4688-b7f5-ea07361b26ab"
#define CHAR_ALARM_STAT_UUID "beb5483e-36e1-4688-b7f5-ea07361b26ac"
#define CHAR_LOGS_UUID "beb5483e-36e1-4688-b7f5-ea07361b26ad"
#define CHAR_CFG_MAX_CURRENT_UUID "beb5483e-36e1-4688-b7f5-ea07361b26ae"
#define CHAR_CFG_MAX_TEMP_PCB_UUID "beb5483e-36e1-4688-b7f5-ea07361b26af"
#define CHAR_CFG_MAX_TEMP_AMB_UUID "beb5483e-36e1-4688-b7f5-ea07361b26b0"

BLEManagerLow::BLEManagerLow() : _server(nullptr), _isConnected(false) {
    _currentConfig = { LimitConfig(), false };
}

void BLEManagerLow::init(const std::string& deviceName) {
    BLEDevice::init(deviceName);
    _server = BLEDevice::createServer();
    _server->setCallbacks(this);

    BLEService* pService = _server->createService(SERVICE_UUID);

    // Initialisation identique à votre fichier...
    _charCurrent = pService->createCharacteristic(CHAR_CURRENT_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
    _charCurrent->addDescriptor(new BLE2902());

    // ... (Instanciez les autres caractéristiques de télémétrie ici) ...

    _charCfgMaxCurrent = pService->createCharacteristic(CHAR_CFG_MAX_CURRENT_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
    _charCfgMaxTempPcb = pService->createCharacteristic(CHAR_CFG_MAX_TEMP_PCB_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);

    _charCfgMaxCurrent->setCallbacks(this);
    _charCfgMaxTempPcb->setCallbacks(this);

    setFloatValue(_charCfgMaxCurrent, _currentConfig.config.current_limit_high);
    setFloatValue(_charCfgMaxTempPcb, _currentConfig.config.temp_pcb_limit_high);

    pService->start();

    BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    BLEDevice::startAdvertising();
}

void BLEManagerLow::updateTelemetry(float current, float pcbTemp, float ambTemp1, float ambTemp2, uint32_t alarmStatus) {
    setFloatValue(_charCurrent, current);
    setFloatValue(_charPcbTemp, pcbTemp);
    // ...
    if (_isConnected) {
        _charCurrent->notify();
        _charPcbTemp->notify();
        // ...
    }
}

void BLEManagerLow::updateLogs(const std::string& logMessage) {
    // ...
}

AppConfig BLEManagerLow::getConfig() const { return _currentConfig; }
void BLEManagerLow::clearUpdateFlag() { _currentConfig.isUpdated = false; }
bool BLEManagerLow::isConnected() const { return _isConnected; }

void BLEManagerLow::setFloatValue(BLECharacteristic* pChar, float value) {
    pChar->setValue((uint8_t*)&value, sizeof(float));
}

void BLEManagerLow::onConnect(BLEServer* pServer) { _isConnected = true; }
void BLEManagerLow::onDisconnect(BLEServer* pServer) {
    _isConnected = false;
    BLEDevice::startAdvertising();
}

void BLEManagerLow::onWrite(BLECharacteristic* pCharacteristic) {
    std::string uuid = pCharacteristic->getUUID().toString();
    std::string rxValue = pCharacteristic->getValue();

    if (rxValue.length() == sizeof(float)) {
        float newValue = *(float*)rxValue.data();

        if (uuid == CHAR_CFG_MAX_CURRENT_UUID) {
            _currentConfig.config.current_limit_high = newValue;
            _currentConfig.isUpdated = true;
        } else if (uuid == CHAR_CFG_MAX_TEMP_PCB_UUID) {
            _currentConfig.config.temp_pcb_limit_high = newValue;
            _currentConfig.isUpdated = true;
        }
    }
}