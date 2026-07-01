#pragma once

#include <NimBLEDevice.h>
#include <string>

#include "Config/config.h"

// UUID du service principal
#define SERVICE_UUID                 "4fafc201-1fb5-459e-8fcc-c5c9c331914b"

// UUIDs Télémétrie (READ | NOTIFY)
#define CHAR_CURRENT_UUID            "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define CHAR_PCB_TEMP_UUID           "beb5483e-36e1-4688-b7f5-ea07361b26a9"
#define CHAR_AMB1_TEMP_UUID           "beb5483e-36e1-4688-b7f5-ea07361b26aa"
#define CHAR_AMB2_TEMP_UUID           "beb5483e-36e1-4688-b7f5-ea07361b26ab"
#define CHAR_ALARM_STAT_UUID         "beb5483e-36e1-4688-b7f5-ea07361b26ac"
#define CHAR_LOGS_UUID               "beb5483e-36e1-4688-b7f5-ea07361b26ad"

// UUIDs Configuration (READ | WRITE)
#define CHAR_CFG_MAX_CURRENT_UUID    "beb5483e-36e1-4688-b7f5-ea07361b26ae"
#define CHAR_CFG_MAX_TEMP_UUID       "beb5483e-36e1-4688-b7f5-ea07361b26af"

struct AppConfig {
    LimitConfig config;
    bool isUpdated; // Flag pour signaler à la boucle principale qu'une config a changé
};

class BLEManager : public NimBLEServerCallbacks, public NimBLECharacteristicCallbacks {
public:
    BLEManager();
    ~BLEManager() = default;

    void init(const std::string& deviceName);

    // Mises à jour des données exposées
    void updateTelemetry(float current, float pcbTemp, float ambTemp1, float ambTemp2, uint32_t alarmStatus);
    void updateLogs(const std::string& logMessage);

    // Gestion de la configuration distante
    AppConfig getConfig() const;
    void clearUpdateFlag();

protected:
    // Callbacks Serveur
    void onConnect(NimBLEServer* pServer) override;
    void onDisconnect(NimBLEServer* pServer) override;

    // Callbacks Caractéristiques (Écriture)
    void onWrite(NimBLECharacteristic* pCharacteristic) override;

private:
    NimBLEServer* _server;

    NimBLECharacteristic* _charCurrent;
    NimBLECharacteristic* _charPcbTemp;
    NimBLECharacteristic* _charAmbTemp1;
    NimBLECharacteristic* _charAmbTemp2;
    NimBLECharacteristic* _charAlarm;
    NimBLECharacteristic* _charLogs;

    NimBLECharacteristic* _charCfgMaxCurrent;
    NimBLECharacteristic* _charCfgMaxTemp;

    bool _isConnected;
    AppConfig _currentConfig;

    // Utilitaire pour définir une valeur float en binaire
    void setFloatValue(NimBLECharacteristic* pChar, float value);
};