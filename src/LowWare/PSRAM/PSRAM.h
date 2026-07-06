//
// Created by Ahliko on 06/07/2026.
//

#ifndef TP_ESP32_REMOTE_TEMPLATE_PSRAM_H
#define TP_ESP32_REMOTE_TEMPLATE_PSRAM_H
#include <Arduino.h>
#include <esp_heap_caps.h>

#include "Interfaces/RAMInterface.h"

class PSRAM : public Print, public RAMInterface{
public:
    explicit PSRAM(size_t capacity);
    ~PSRAM() override;

    bool init() override;

    size_t write(uint8_t c) override;
    size_t write(const uint8_t *buffer, size_t size) override;

    String getAllLogs();
    void dumpTo(Print& destination);

    void clear() override;
    bool isEmpty() const override;
    size_t available() const override;

private:
    char* _buffer;
    size_t _capacity;
    size_t _head;
    size_t _tail;
    bool _isFull;
};


#endif // TP_ESP32_REMOTE_TEMPLATE_PSRAM_H
