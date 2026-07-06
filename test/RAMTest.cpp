#include "RAMTest.h"

RAMTest::RAMTest(size_t capacity) : _buffer(nullptr), _capacity(capacity), _head(0), _tail(0), _isFull(false) {}

RAMTest::~RAMTest() {
    if (_buffer) {
        free(_buffer);
    }
}

bool RAMTest::init() {
    // Standard host malloc for testing
    _buffer = (char*)malloc(_capacity);
    if (!_buffer) return false;
    clear();
    return true;
}

size_t RAMTest::write(uint8_t c) {
    if (!_buffer) return 0;

    _buffer[_head] = c;
    _head = (_head + 1) % _capacity;

    if (_isFull) {
        _tail = (_tail + 1) % _capacity;
    } else if (_head == _tail) {
        _isFull = true;
    }

    return 1;
}

size_t RAMTest::write(const uint8_t *buffer, size_t size) {
    if (!_buffer) return 0;

    size_t bytesWritten = 0;
    while (size--) {
        write(*buffer++);
        bytesWritten++;
    }
    return bytesWritten;
}

void RAMTest::clear() {
    _head = 0;
    _tail = 0;
    _isFull = false;
    if (_buffer) memset(_buffer, 0, _capacity);
}

bool RAMTest::isEmpty() const {
    return (!_isFull && (_head == _tail));
}

size_t RAMTest::available() const {
    if (_isFull) return _capacity;
    if (_head >= _tail) return _head - _tail;
    return _capacity - _tail + _head;
}
