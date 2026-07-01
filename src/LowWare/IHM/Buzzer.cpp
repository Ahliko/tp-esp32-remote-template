#include "Buzzer.h"

// ─────────────────────────────────────────────────────────────────────────────
//  Mélodies prédéfinies
// ─────────────────────────────────────────────────────────────────────────────

static const MelodyNote _startupNotes[] = {
        {Note::C5, 80, 10},
        {Note::E5, 80, 10},
        {Note::G5, 80, 10},
        {Note::C6, 160, 0},
};
const Melody MELODY_STARTUP = {_startupNotes, 4, 0};

static const MelodyNote _successNotes[] = {
        {Note::G5, 120, 20},
        {Note::G5, 120, 20},
        {Note::A5, 120, 20},
        {Note::G5, 240, 0},
};
const Melody MELODY_SUCCESS = {_successNotes, 4, 0};

static const MelodyNote _errorNotes[] = {
        {Note::G4, 200, 30},
        {Note::DS4, 200, 30},
        {Note::C4, 400, 0},
};
const Melody MELODY_ERROR = {_errorNotes, 3, 0};

static const MelodyNote _alarmNotes[] = {
        {Note::ALARM_HIGH, 500, 10},
        {Note::ALARM_LOW, 500, 10},
};
const Melody MELODY_ALARM = {_alarmNotes, 2, 0}; // repeat via playAlarm()

static const MelodyNote _clickNote[] = {
        {Note::CLICK, 15, 0},
};
const Melody MELODY_CLICK = {_clickNote, 1, 0};

static const MelodyNote _doubleBeepNotes[] = {
        {Note::BEEP_STD, 100, 100},
        {Note::BEEP_STD, 100, 0},
};
const Melody MELODY_DOUBLE_BEEP = {_doubleBeepNotes, 2, 0};

// ─────────────────────────────────────────────────────────────────────────────
//  Constructeur & begin
// ─────────────────────────────────────────────────────────────────────────────

Buzzer::Buzzer(uint8_t pin, uint8_t channel, bool passive) : _pin(pin), _channel(channel % 8), _passive(passive) {}

void Buzzer::init() const {
    if (_passive) {
        ledcSetup(_channel, Note::BEEP_STD, 10);
        ledcAttachPin(_pin, _channel);
        ledcWrite(_channel, 0);
    } else {
        pinMode(_pin, OUTPUT);
        digitalWrite(_pin, LOW);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  tone / noTone
// ─────────────────────────────────────────────────────────────────────────────

void Buzzer::tone(uint32_t freq_hz) { _startFreq(freq_hz); }

void Buzzer::noTone() { _stopFreq(); }

// ─────────────────────────────────────────────────────────────────────────────
//  Beep simple / beepN non-bloquants
// ─────────────────────────────────────────────────────────────────────────────

void Buzzer::beep(uint32_t freq_hz, uint32_t dur_ms) { beepN(1, freq_hz, dur_ms, 0); }

void Buzzer::beepN(uint8_t times, uint32_t freq_hz, uint32_t dur_ms, uint32_t gap_ms) {
    stop();
    _simpleBeep = true;
    _melody = nullptr;
    _alarm = false;
    _beepFreq = freq_hz;
    _beepDur = dur_ms;
    _beepGap = gap_ms;
    _beepTimes = times;
    _beepCount = 0;
    _inGap = false;
    _playing = true;
    _lastTime = millis();
    _startFreq(freq_hz);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Mélodies
// ─────────────────────────────────────────────────────────────────────────────

void Buzzer::play(const Melody &melody) {
    stop();
    _melody = &melody;
    _simpleBeep = false;
    _alarm = false;
    _noteIndex = 0;
    _repeatCount = 0;
    _inGap = false;
    _playing = true;
    _lastTime = millis();
    _startFreq(melody.notes[0].freq_hz);
}

void Buzzer::playAlarm() {
    stop();
    _alarm = true;
    _playing = true;
    _alarmPhase = 0;
    _alarmLastTime = millis();
    _startFreq(Note::ALARM_HIGH);
}

void Buzzer::stop() {
    _playing = false;
    _simpleBeep = false;
    _alarm = false;
    _melody = nullptr;
    _stopFreq();
}

// ─────────────────────────────────────────────────────────────────────────────
//  setVolume
// ─────────────────────────────────────────────────────────────────────────────

void Buzzer::setVolume(uint8_t duty_pct) {
    _volume = constrain(duty_pct, 1, 99);
    if (_active && _passive) {
        ledcWrite(_channel, _dutyValue());
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  update() — à appeler dans loop()
// ─────────────────────────────────────────────────────────────────────────────

bool Buzzer::update() {
    if (!_playing)
        return false;
    bool changed = false;
    uint32_t now = millis();

    // ── Alarme bitonale ──────────────────────────────────────────────────────
    if (_alarm) {
        if (now - _alarmLastTime >= 500) {
            _alarmPhase = _alarmPhase == 0 ? 1 : 0;
            _alarmLastTime = now;
            _startFreq(_alarmPhase == 0 ? Note::ALARM_HIGH : Note::ALARM_LOW);
            changed = true;
        }
        return changed;
    }

    // ── Beep simple ──────────────────────────────────────────────────────────
    if (_simpleBeep) {
        uint32_t elapsed = now - _lastTime;
        if (!_inGap && elapsed >= _beepDur) {
            _stopFreq();
            if (_beepGap == 0 || _beepCount + 1 >= _beepTimes) {
                _beepCount++;
                if (_beepTimes != 0 && _beepCount >= _beepTimes) {
                    _playing = false;
                    _simpleBeep = false;
                    return true;
                }
            }
            if (_beepGap > 0) {
                _inGap = true;
                _lastTime = now;
            } else {
                // pas de gap → enchaîner
                _startFreq(_beepFreq);
                _lastTime = now;
            }
            changed = true;
        } else if (_inGap && elapsed >= _beepGap) {
            _beepCount++;
            if (_beepTimes != 0 && _beepCount >= _beepTimes) {
                _playing = false;
                _simpleBeep = false;
                return true;
            }
            _inGap = false;
            _lastTime = now;
            _startFreq(_beepFreq);
            changed = true;
        }
        return changed;
    }

    // ── Mélodie ──────────────────────────────────────────────────────────────
    if (_melody) {
        const MelodyNote &note = _melody->notes[_noteIndex];
        uint32_t elapsed = now - _lastTime;

        if (!_inGap && elapsed >= note.dur_ms) {
            _stopFreq();
            if (note.gap_ms > 0) {
                _inGap = true;
                _lastTime = now;
            } else {
                goto next_note;
            }
            changed = true;
        } else if (_inGap && elapsed >= note.gap_ms) {
        next_note:
            _inGap = false;
            _noteIndex++;
            if (_noteIndex >= _melody->count) {
                _noteIndex = 0;
                if (_melody->repeat != 0) {
                    _repeatCount++;
                    if (_repeatCount >= _melody->repeat) {
                        _playing = false;
                        _melody = nullptr;
                        return true;
                    }
                }
            }
            _lastTime = now;
            _startFreq(_melody->notes[_noteIndex].freq_hz);
            changed = true;
        }
    }

    return changed;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Privé
// ─────────────────────────────────────────────────────────────────────────────

void Buzzer::_startFreq(uint32_t freq_hz) {
    _currentFreq = freq_hz;
    if (_passive) {
        if (freq_hz == 0) {
            ledcWrite(_channel, 0);
        } else {
            ledcChangeFrequency(_channel, freq_hz, 10);
            ledcWrite(_channel, _dutyValue());
        }
    } else {
        digitalWrite(_pin, HIGH);
    }
    _active = freq_hz > 0;
}

void Buzzer::_stopFreq() {
    _currentFreq = 0;
    _active = false;
    if (_passive) {
        ledcWrite(_channel, 0);
    } else {
        digitalWrite(_pin, LOW);
    }
}

uint32_t Buzzer::_dutyValue() const {
    // 10-bit PWM → 1023 max; volume 50% → duty carré pur
    return static_cast<uint32_t>(_volume * 1023 / 100);
}
