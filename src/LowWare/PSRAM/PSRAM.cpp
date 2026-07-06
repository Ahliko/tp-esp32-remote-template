#include "PSRAM.h"

PSRAM::PSRAM(size_t capacity) : _buffer(nullptr), _capacity(capacity), _head(0), _tail(0), _isFull(false) {}

PSRAM::~PSRAM() {
    if (_buffer) {
        heap_caps_free(_buffer);
    }
}

bool PSRAM::init() {
    if (!psramFound()) {
        Serial.println("[LogBuffer] PSRAM non trouvée !");
        return false;
    }

    _buffer = (char *) heap_caps_malloc(_capacity, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);

    if (_buffer == nullptr) {
        Serial.println("[LogBuffer] Échec allocation PSRAM");
        return false;
    }

    clear();
    return true;
}

size_t PSRAM::write(uint8_t c) {
    if (!_buffer)
        return 0;

    _buffer[_head] = c;
    _head = (_head + 1) % _capacity;

    if (_isFull) {
        _tail = (_tail + 1) % _capacity;
    } else if (_head == _tail) {
        _isFull = true;
    }

    return 1;
}

size_t PSRAM::write(const uint8_t *buffer, size_t size) {
    if (!_buffer)
        return 0;

    size_t bytesWritten = 0;
    while (size--) {
        write(*buffer++);
        bytesWritten++;
    }
    return bytesWritten;
}

String PSRAM::getAllLogs() {
    if (!_buffer || isEmpty())
        return String("");

    String result;
    result.reserve(available());

    size_t i = _tail;
    do {
        result += _buffer[i];
        i = (i + 1) % _capacity;
    } while (i != _head);

    return result;
}

void PSRAM::dumpTo(Print &destination) {
    if (!_buffer || isEmpty())
        return;

    size_t i = _tail;
    do {
        destination.write(_buffer[i]);
        i = (i + 1) % _capacity;
    } while (i != _head);
}

void PSRAM::clear() {
    _head = 0;
    _tail = 0;
    _isFull = false;
    if (_buffer)
        memset(_buffer, 0, _capacity);
}

bool PSRAM::isEmpty() const { return (!_isFull && (_head == _tail)); }

size_t PSRAM::available() const {
    if (_isFull)
        return _capacity;
    if (_head >= _tail)
        return _head - _tail;
    return _capacity - _tail + _head;
}
