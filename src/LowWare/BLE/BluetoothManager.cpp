#include "LowWare/BLE/BluetoothManager.h"

BLEManager::BLEManager() : _server(nullptr), _isConnected(false) {
    // Valeurs par défaut au démarrage
    _currentConfig = { LimitConfig(), false };
}

void BLEManager::init(const std::string& deviceName) {
    NimBLEDevice::init(deviceName);
    NimBLEDevice::setPower(ESP_PWR_LVL_P9); // Puissance TX maximale

    _server = NimBLEDevice::createServer();
    _server->setCallbacks(this);

    NimBLEService* pService = _server->createService(SERVICE_UUID);

    // --- Instanciation des caractéristiques de Télémétrie ---
    _charCurrent = pService->createCharacteristic(CHAR_CURRENT_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
    _charPcbTemp = pService->createCharacteristic(CHAR_PCB_TEMP_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
    _charAmbTemp = pService->createCharacteristic(CHAR_AMB_TEMP_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
    _charAlarm   = pService->createCharacteristic(CHAR_ALARM_STAT_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
    _charLogs    = pService->createCharacteristic(CHAR_LOGS_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);

    // --- Instanciation des caractéristiques de Configuration ---
    _charCfgMaxCurrent = pService->createCharacteristic(CHAR_CFG_MAX_CURRENT_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
    _charCfgMaxTemp    = pService->createCharacteristic(CHAR_CFG_MAX_TEMP_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);

    // Assigner les callbacks d'écriture pour la config
    _charCfgMaxCurrent->setCallbacks(this);
    _charCfgMaxTemp->setCallbacks(this);

    // Initialisation des valeurs par défaut dans les caractéristiques
    setFloatValue(_charCfgMaxCurrent, _currentConfig.config.current_limit_high);
    setFloatValue(_charCfgMaxTemp, _currentConfig.config.temp_limit_high);

    pService->start();

    // Démarrer l'Advertising
    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->start();
}

void BLEManager::updateTelemetry(float current, float pcbTemp, float ambTemp, uint32_t alarmStatus) {
    Serial.println("telemetry");
    Serial.println(ambTemp);
    setFloatValue(_charCurrent, current);
    setFloatValue(_charPcbTemp, pcbTemp);
    setFloatValue(_charAmbTemp, ambTemp);

    _charAlarm->setValue((uint8_t*)&alarmStatus, sizeof(alarmStatus));

    if (_isConnected) {
        _charCurrent->notify();
        _charPcbTemp->notify();
        _charAmbTemp->notify();
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

void BLEManager::setFloatValue(NimBLECharacteristic* pChar, float value) {
    pChar->setValue((uint8_t*)&value, sizeof(float));
}

// ─────────────────────────────────────────────────────────────────────────────
// Callbacks
// ─────────────────────────────────────────────────────────────────────────────

void BLEManager::onConnect(NimBLEServer* pServer) {
    _isConnected = true;
    NimBLEDevice::startAdvertising(); // Permet à d'autres appareils de voir le device
}

void BLEManager::onDisconnect(NimBLEServer* pServer) {
    _isConnected = false;
    NimBLEDevice::startAdvertising(); // Relance l'advertising pour reconnexion
}

std::string uint8ToHex(uint8_t value) {
    char buffer[3]; // 2 caractères pour l'hexa + 1 pour le caractère de fin de chaîne '\0'
    snprintf(buffer, sizeof(buffer), "%02X", value);
    return std::string(buffer);
}

std::string arrayToHex(const uint8_t* data, size_t length) {
    std::string result;
    result.reserve(length * 2); // Optimisation mémoire

    char buffer[3];
    for (size_t i = 0; i < length; i++) {
        snprintf(buffer, sizeof(buffer), "%02X", data[i]);
        result += buffer;
    }

    return result;
}

void BLEManager::onWrite(NimBLECharacteristic* pCharacteristic) {
    std::string uuid = pCharacteristic->getUUID().toString();

    // Récupération de la donnée brute et cast en float
    if (pCharacteristic->getDataLength() == sizeof(float)) {
        float newValue = *(float*)pCharacteristic->getValue().data();

        if (uuid == CHAR_CFG_MAX_CURRENT_UUID) {
            _currentConfig.config.current_limit_high = newValue;
            _currentConfig.isUpdated = true;
        } else if (uuid == CHAR_CFG_MAX_TEMP_UUID) {
            _currentConfig.config.temp_limit_high = newValue;
            _currentConfig.isUpdated = true;
        }
    }
}