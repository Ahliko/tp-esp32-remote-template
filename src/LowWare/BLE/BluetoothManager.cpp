#include "LowWare/BLE/BluetoothManager.h"
#include <Arduino.h>

// Si vous avez absolument besoin de régler la puissance, décommentez cet include et la fonction dans init()
// #include "esp_bt.h"

BLEManager::BLEManager() : _server(nullptr), _isConnected(false) {
    // Valeurs par défaut au démarrage
    _currentConfig = { LimitConfig(), false };
}

void BLEManager::init(const std::string& deviceName) {
    BLEDevice::init(deviceName);

    // Avec la lib BLE classique, la modification de la puissance se fait via le framework ESP-IDF :
    // esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_P9);

    _server = BLEDevice::createServer();
    _server->setCallbacks(this);

    BLEService* pService = _server->createService(SERVICE_UUID);

    // --- Instanciation des caractéristiques de Télémétrie ---
    // Note : l'ajout du descripteur BLE2902 est obligatoire pour que le NOTIFY fonctionne correctement.
    _charCurrent = pService->createCharacteristic(CHAR_CURRENT_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
    _charCurrent->addDescriptor(new BLE2902());

    _charPcbTemp = pService->createCharacteristic(CHAR_PCB_TEMP_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
    _charPcbTemp->addDescriptor(new BLE2902());

    _charAmbTemp1 = pService->createCharacteristic(CHAR_AMB1_TEMP_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
    _charAmbTemp1->addDescriptor(new BLE2902());

    _charAmbTemp2 = pService->createCharacteristic(CHAR_AMB2_TEMP_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
    _charAmbTemp2->addDescriptor(new BLE2902());

    _charAlarm = pService->createCharacteristic(CHAR_ALARM_STAT_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
    _charAlarm->addDescriptor(new BLE2902());

    _charLogs = pService->createCharacteristic(CHAR_LOGS_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
    _charLogs->addDescriptor(new BLE2902());

    // --- Instanciation des caractéristiques de Configuration ---
    _charCfgMaxCurrent = pService->createCharacteristic(CHAR_CFG_MAX_CURRENT_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
    _charCfgMaxTempPcb = pService->createCharacteristic(CHAR_CFG_MAX_TEMP_PCB_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
    _charCfgMaxTempAmb = pService->createCharacteristic(CHAR_CFG_MAX_TEMP_AMB_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);

    // Assigner les callbacks d'écriture pour la config
    _charCfgMaxCurrent->setCallbacks(this);
    _charCfgMaxTempPcb->setCallbacks(this);

    // Initialisation des valeurs par défaut dans les caractéristiques
    setFloatValue(_charCfgMaxCurrent, _currentConfig.config.current_limit_high);
    setFloatValue(_charCfgMaxTempPcb, _currentConfig.config.temp_pcb_limit_high);

    pService->start();

    // Démarrer l'Advertising
    BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    // Aide à la compatibilité pour certains clients iOS
    pAdvertising->setMinPreferred(0x06);
    pAdvertising->setMinPreferred(0x12);

    BLEDevice::startAdvertising();
}

void BLEManager::updateTelemetry(float current, float pcbTemp, float ambTemp1, float ambTemp2, uint32_t alarmStatus) {
    Serial.println("telemetry");
    Serial.println(ambTemp1);

    setFloatValue(_charCurrent, current);
    setFloatValue(_charPcbTemp, pcbTemp);
    setFloatValue(_charAmbTemp1, ambTemp1);
    setFloatValue(_charAmbTemp2, ambTemp2);

    _charAlarm->setValue((uint8_t*)&alarmStatus, sizeof(alarmStatus));

    if (_isConnected) {
        _charCurrent->notify();
        _charPcbTemp->notify();
        _charAmbTemp1->notify();
        _charAmbTemp2->notify();
        _charAlarm->notify();
    }
}

void BLEManager::updateLogs(const std::string& logMessage) {
    _charLogs->setValue(logMessage);
    if (_isConnected) {
        _charLogs->notify();
    }
}

AppConfig BLEManager::getConfig() const {
    return _currentConfig;
}

void BLEManager::clearUpdateFlag() {
    _currentConfig.isUpdated = false;
}

void BLEManager::setFloatValue(BLECharacteristic* pChar, float value) {
    pChar->setValue((uint8_t*)&value, sizeof(float));
}

// ─────────────────────────────────────────────────────────────────────────────
// Callbacks
// ─────────────────────────────────────────────────────────────────────────────

void BLEManager::onConnect(BLEServer* pServer) {
    _isConnected = true;
}

void BLEManager::onDisconnect(BLEServer* pServer) {
    _isConnected = false;
    // Relance l'advertising pour la reconnexion avec la librairie standard
    BLEDevice::startAdvertising();
}

std::string uint8ToHex(uint8_t value) {
    char buffer[3];
    snprintf(buffer, sizeof(buffer), "%02X", value);
    return std::string(buffer);
}

std::string arrayToHex(const uint8_t* data, size_t length) {
    std::string result;
    result.reserve(length * 2);

    char buffer[3];
    for (size_t i = 0; i < length; i++) {
        snprintf(buffer, sizeof(buffer), "%02X", data[i]);
        result += buffer;
    }

    return result;
}

void BLEManager::onWrite(BLECharacteristic* pCharacteristic) {
    std::string uuid = pCharacteristic->getUUID().toString();

    // Avec la lib standard, la méthode pour récupérer les données utilise une std::string
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