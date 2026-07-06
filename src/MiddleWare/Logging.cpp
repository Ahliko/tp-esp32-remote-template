//
// Created by Ahliko on 06/07/2026.
//

#include "Logging.h"

Logging::Logging(RAMInterface &ram_buffer) : m_ram_buffer(ram_buffer) {}

void Logging::init() {
    m_ram_buffer.init();
    logSystemEvent(millis(), "BOOT", "Démarrage du système AGV");
}

void Logging::writeLog(const char *prefix, uint32_t timestamp, const char *message) {
    // Format: [TIMESTAMP] [PREFIX] MESSAGE\n
    char buffer[128];
    snprintf(buffer, sizeof(buffer), "[%lu] [%s] %s\n", timestamp, prefix, message);
    m_ram_buffer.write(reinterpret_cast<const uint8_t *>(buffer), strlen(buffer));
}

void Logging::logMeasurement(uint32_t timestamp, float current, float temperature) {
    char msg[64];
    snprintf(msg, sizeof(msg), "I:%.2fA T:%.1fC", current, temperature);
    writeLog("MESURE", timestamp, msg);
}

void Logging::logAlarm(uint32_t timestamp, const String &reason, float triggerValue) {
    char msg[64];
    snprintf(msg, sizeof(msg), "%s (Valeur: %.2f)", reason.c_str(), triggerValue);
    writeLog("ALARME", timestamp, msg);
}

void Logging::logSystemEvent(uint32_t timestamp, const String &eventType, const String &details) {
    char msg[128];
    snprintf(msg, sizeof(msg), "%s - %s", eventType.c_str(), details.c_str());
    writeLog("SYSTEME", timestamp, msg);
}
