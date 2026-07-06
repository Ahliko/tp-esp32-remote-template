//
// Created by Ahliko on 06/07/2026.
//

#ifndef TP_ESP32_REMOTE_TEMPLATE_RAMINTERFACE_H
#define TP_ESP32_REMOTE_TEMPLATE_RAMINTERFACE_H
#include <Arduino.h>
class RAMInterface {
    public:
    virtual	~RAMInterface() = default;
    virtual	bool init() = 0;
    virtual size_t write(const uint8_t *buffer, size_t size) = 0;
    virtual void clear() = 0;
    virtual bool isEmpty() const = 0;
    virtual size_t available() const = 0;
};
#endif // TP_ESP32_REMOTE_TEMPLATE_RAMINTERFACE_H
