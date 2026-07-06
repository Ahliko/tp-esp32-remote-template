//
// Created by Ahliko on 06/07/2026.
//

#ifndef TP_ESP32_REMOTE_TEMPLATE_LOGGING_H
#define TP_ESP32_REMOTE_TEMPLATE_LOGGING_H
#include "Interfaces/RAMInterface.h"

class Logging {
public:
    explicit Logging(RAMInterface &ram_buffer);
    ~Logging() = default;

    void init();

    void logMeasurement(uint32_t timestamp, float current, float temperature);

    void logAlarm(uint32_t timestamp, const String &reason, float triggerValue);

    void logSystemEvent(uint32_t timestamp, const String &eventType, const String &details);

private:
    RAMInterface &m_ram_buffer;

    // Fonction utilitaire interne pour écrire dans la RAM
    void writeLog(const char *prefix, uint32_t timestamp, const char *message);
};


#endif // TP_ESP32_REMOTE_TEMPLATE_LOGGING_H
