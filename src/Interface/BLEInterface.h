#pragma once
#include <cstdint>
#include <string>
#include "Config/config.h"

// Structure de retour pour la configuration
struct AppConfig {
    LimitConfig config;
    bool isUpdated;
};

class BLEInterface {
public:
    virtual ~BLEInterface() = default;

    /**
     * @brief Initialise le périphérique BLE et démarre l'advertising
     */
    virtual void init(const std::string &deviceName) = 0;

    /**
     * @brief Met à jour les valeurs de télémétrie et notifie les clients connectés
     */
    virtual void updateTelemetry(float current, float pcbTemp, float ambTemp1, float ambTemp2,
                                 uint32_t alarmStatus) = 0;

    /**
     * @brief Envoie un message de log via notification BLE
     */
    virtual void updateLogs(const std::string &logMessage) = 0;

    /**
     * @brief Récupère la dernière configuration reçue via BLE
     */
    virtual AppConfig getConfig() const = 0;

    /**
     * @brief Acquitte la réception d'une nouvelle configuration
     */
    virtual void clearUpdateFlag() = 0;

    /**
     * @brief Indique si un client (ex: smartphone) est actuellement connecté
     */
    virtual bool isConnected() const = 0;
};
