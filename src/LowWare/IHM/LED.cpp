#include "LED.h"

// ─────────────────────────────────────────────────────────────────────────────
//  Patterns prédéfinis
// ─────────────────────────────────────────────────────────────────────────────

// SOS : ... --- ...
static const LEDStep _sosSteps[] = {
        {100, 100}, {100, 100}, {100, 300}, // S : 3 courts
        {300, 100}, {300, 100}, {300, 300}, // O : 3 longs
        {100, 100}, {100, 100}, {100, 700}, // S : 3 courts
};
const LEDPattern LED_PATTERN_SOS = {_sosSteps, 9, 0};

// Heartbeat : double flash rapide
static const LEDStep _heartbeatSteps[] = {
        {80, 80},
        {80, 700},
};
const LEDPattern LED_PATTERN_HEARTBEAT = {_heartbeatSteps, 2, 0};

// Slow blink
static const LEDStep _slowSteps[] = {{800, 800}};
const LEDPattern LED_PATTERN_SLOW_BLINK = {_slowSteps, 1, 0};

// Fast blink
static const LEDStep _fastSteps[] = {{100, 100}};
const LEDPattern LED_PATTERN_FAST_BLINK = {_fastSteps, 1, 0};

// ─────────────────────────────────────────────────────────────────────────────
//  Constructeur & begin
// ─────────────────────────────────────────────────────────────────────────────

LED::LED(uint8_t pin, LEDMode mode, uint8_t channel, bool inverted) :
    _pin(pin), _mode(mode), _channel(channel % LED_MAX_CHANNEL), _inverted(inverted) {}

void LED::begin() {
    if (_mode == LEDMode::PWM) {
        ledcSetup(_channel, LED_PWM_FREQ_HZ, LED_PWM_BITS);
        ledcAttachPin(_pin, _channel);
        ledcWrite(_channel, 0);
    } else {
        pinMode(_pin, OUTPUT);
        digitalWrite(_pin, _inverted ? HIGH : LOW);
    }
    _state = false;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Contrôle de base
// ─────────────────────────────────────────────────────────────────────────────

void LED::on() {
    _fading = false;
    _running = false;
    _writeState(true);
}

void LED::off() {
    _fading = false;
    _running = false;
    _writeState(false);
}

void LED::toggle() { _state ? off() : on(); }

void LED::setIntensity(float intensity) {
    _intensity = constrain(intensity, 0.0f, 1.0f);
    if (_mode == LEDMode::PWM) {
        _writeDuty(_intensity);
        _state = (_intensity > 0.0f);
    }
}

void LED::fadeTo(float targetIntensity, uint32_t duration_ms) {
    if (_mode != LEDMode::PWM)
        return;
    _fadeStart = _intensity;
    _fadeTarget = constrain(targetIntensity, 0.0f, 1.0f);
    _fadeDuration = duration_ms;
    _fadeStartTime = millis();
    _fading = true;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Blink bloquant
// ─────────────────────────────────────────────────────────────────────────────

void LED::blinkBlocking(uint8_t times, uint32_t on_ms, uint32_t off_ms) {
    for (uint8_t i = 0; i < times; i++) {
        _writeState(true);
        delay(on_ms);
        _writeState(false);
        if (i < times - 1)
            delay(off_ms);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Blink / pattern non-bloquant
// ─────────────────────────────────────────────────────────────────────────────

void LED::startBlink(uint32_t on_ms, uint32_t off_ms, uint8_t times) {
    _pattern = nullptr;
    _blinkOnMs = on_ms;
    _blinkOffMs = off_ms;
    _blinkTimes = times;
    _blinkCount = 0;
    _running = true;
    _inOnPhase = false; // démarre par une phase ON
    _lastTime = millis();
    _writeState(false);
}

void LED::startPattern(const LEDPattern &pattern) {
    _pattern = &pattern;
    _stepIndex = 0;
    _repeatCount = 0;
    _running = true;
    _inOnPhase = false;
    _lastTime = millis();
    _writeState(false);
}

void LED::startSOS() { startPattern(LED_PATTERN_SOS); }

void LED::startHeartbeat() { startPattern(LED_PATTERN_HEARTBEAT); }

void LED::stop() {
    _running = false;
    _fading = false;
    _writeState(false);
}

// ─────────────────────────────────────────────────────────────────────────────
//  update() — à appeler dans loop()
// ─────────────────────────────────────────────────────────────────────────────

bool LED::update() {
    bool changed = false;

    // ── Fade ────────────────────────────────────────────────────────────────
    if (_fading) {
        uint32_t elapsed = millis() - _fadeStartTime;
        if (elapsed >= _fadeDuration) {
            setIntensity(_fadeTarget);
            _fading = false;
            changed = true;
        } else {
            float t = static_cast<float>(elapsed) / static_cast<float>(_fadeDuration);
            setIntensity(_fadeStart + (_fadeTarget - _fadeStart) * t);
            changed = true;
        }
    }

    // ── Blink / Pattern ─────────────────────────────────────────────────────
    if (!_running)
        return changed;

    uint32_t now = millis();
    uint32_t elapsed = now - _lastTime;
    uint32_t onMs, offMs;

    if (_pattern) {
        const LEDStep &step = _pattern->steps[_stepIndex];
        onMs = step.on_ms;
        offMs = step.off_ms;
    } else {
        onMs = _blinkOnMs;
        offMs = _blinkOffMs;
    }

    if (!_inOnPhase && elapsed >= offMs) {
        // → ON
        _inOnPhase = true;
        _lastTime = now;
        _writeState(true);
        changed = true;
    } else if (_inOnPhase && elapsed >= onMs) {
        // → OFF
        _inOnPhase = false;
        _lastTime = now;
        _writeState(false);
        changed = true;

        // Avancer step si pattern
        if (_pattern) {
            _stepIndex++;
            if (_stepIndex >= _pattern->count) {
                _stepIndex = 0;
                if (_pattern->repeat != 0) {
                    _repeatCount++;
                    if (_repeatCount >= _pattern->repeat) {
                        _running = false;
                        return true;
                    }
                }
            }
        } else {
            // Blink simple
            if (_blinkTimes != 0) {
                _blinkCount++;
                if (_blinkCount >= _blinkTimes) {
                    _running = false;
                }
            }
        }
    }

    return changed;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Privé
// ─────────────────────────────────────────────────────────────────────────────

void LED::_writeState(bool on) {
    bool prev = _state;
    _state = on;
    if (_mode == LEDMode::PWM) {
        _writeDuty(on ? _intensity : 0.0f);
    } else {
        bool level = _inverted ? !on : on;
        digitalWrite(_pin, level ? HIGH : LOW);
    }
    if (_state != prev)
        _notifyChange(_state);
}

void LED::_writeDuty(float intensity) {
    uint32_t duty = static_cast<uint32_t>(intensity * LED_PWM_MAX_DUTY);
    ledcWrite(_channel, duty);
}

void LED::_notifyChange(bool newState) {
    if (_callback)
        _callback(newState);
}
