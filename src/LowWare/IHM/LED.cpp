#include "LED.h"

LED::LED(uint8_t pin, bool inverted) : _pin(pin), _inverted(inverted) {}

void LED::init() {
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, _inverted ? HIGH : LOW);
    _state = false;
}

void LED::on() {
    _running = false;
    _writeState(true);
}

void LED::off() {
    _running = false;
    _writeState(false);
}

void LED::toggle() { _state ? off() : on(); }

void LED::blinkBlocking(uint8_t times, uint32_t on_ms, uint32_t off_ms) {
    for (uint8_t i = 0; i < times; i++) {
        _writeState(true);
        delay(on_ms);
        _writeState(false);
        if (i < times - 1)
            delay(off_ms);
    }
}

void LED::startBlink(uint32_t on_ms, uint32_t off_ms, uint8_t times) {
    _blinkOnMs = on_ms;
    _blinkOffMs = off_ms;
    _blinkTimes = times;
    _blinkCount = 0;
    _running = true;
    _inOnPhase = false;
    _lastTime = millis();
    _writeState(false);
}

void LED::stop() {
    _running = false;
    _writeState(false);
}

bool LED::update() {
    bool changed = false;

    if (!_running)
        return changed;

    uint32_t now = millis();
    uint32_t elapsed = now - _lastTime;
    uint32_t onMs, offMs;

    onMs = _blinkOnMs;
    offMs = _blinkOffMs;

    if (!_inOnPhase && elapsed >= offMs) {
        _inOnPhase = true;
        _lastTime = now;
        _writeState(true);
        changed = true;
    } else if (_inOnPhase && elapsed >= onMs) {
        _inOnPhase = false;
        _lastTime = now;
        _writeState(false);
        changed = true;

        if (_blinkTimes != 0) {
            _blinkCount++;
            if (_blinkCount >= _blinkTimes) {
                _running = false;
            }
        }
    }

    return changed;
}

void LED::_writeState(bool on) {
    _state = on;
    bool level = _inverted ? !on : on;
    digitalWrite(_pin, level ? HIGH : LOW);
}
