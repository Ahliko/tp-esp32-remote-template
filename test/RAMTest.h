#ifndef TEST_RAMTEST_H
#define TEST_RAMTEST_H

#include "Interfaces/RAMInterface.h"
#include <string.h>
#include <stdlib.h>

/**
 * RAMTest
 * A mock implementation of RAMInterface for unit testing.
 * Uses standard malloc/free on the host machine instead of ESP32 PSRAM.
 */
class RAMTest : public RAMInterface {
public:
    explicit RAMTest(size_t capacity);
    ~RAMTest() override;

    bool init() override;

    size_t write(uint8_t c); // Used internally, not part of interface
    size_t write(const uint8_t *buffer, size_t size) override;

    void clear() override;
    bool isEmpty() const override;
    size_t available() const override;

    // Helper for testing to retrieve written content
    const char* getBuffer() const { return _buffer; }
    size_t getHead() const { return _head; }
    size_t getTail() const { return _tail; }
    bool isFull() const { return _isFull; }

private:
    char* _buffer;
    size_t _capacity;
    size_t _head;
    size_t _tail;
    bool _isFull;
};

#endif // TEST_RAMTEST_H
