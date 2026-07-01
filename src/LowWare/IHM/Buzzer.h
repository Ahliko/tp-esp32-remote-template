#pragma once
/**
 * @file Buzzer.h
 * @brief Driver C++ pour buzzer passif/actif sur ESP32 via LEDC
 *
 * Buzzer PASSIF : nécessite une fréquence (LEDC génère la tonalité)
 * Buzzer ACTIF  : on/off simple (marche à n'importe quelle fréquence)
 *
 * Fonctionnalités :
 *   - tone() / noTone()   (API compatible Arduino)
 *   - beep() non-bloquant
 *   - Séquences de beeps (mélodies ou alertes)
 *   - Notes musicales prédéfinies
 *   - Mélodies intégrées (startup, erreur, succès, alarme)
 *   - update() pour fonctionnement non-bloquant
 */

#include <Arduino.h>
#include <cstdint>

// ─────────────────────────────────────────────
//  Notes (fréquences en Hz — octave 4)
// ─────────────────────────────────────────────
namespace Note {
    constexpr uint32_t SILENCE = 0;
    constexpr uint32_t C4 = 262;
    constexpr uint32_t CS4 = 277;
    constexpr uint32_t D4 = 294;
    constexpr uint32_t DS4 = 311;
    constexpr uint32_t E4 = 330;
    constexpr uint32_t F4 = 349;
    constexpr uint32_t FS4 = 370;
    constexpr uint32_t G4 = 392;
    constexpr uint32_t GS4 = 415;
    constexpr uint32_t A4 = 440;
    constexpr uint32_t AS4 = 466;
    constexpr uint32_t B4 = 494;

    constexpr uint32_t C5 = 523;
    constexpr uint32_t D5 = 587;
    constexpr uint32_t E5 = 659;
    constexpr uint32_t F5 = 698;
    constexpr uint32_t G5 = 784;
    constexpr uint32_t A5 = 880;
    constexpr uint32_t B5 = 988;

    constexpr uint32_t C6 = 1047;
    constexpr uint32_t E6 = 1319;
    constexpr uint32_t G6 = 1568;

    // Fréquences d'alerte communes
    constexpr uint32_t ALARM_LOW = 800;
    constexpr uint32_t ALARM_HIGH = 2400;
    constexpr uint32_t BEEP_STD = 1000;
    constexpr uint32_t CLICK = 3000;
} // namespace Note

// ─────────────────────────────────────────────
//  Structure d'une note de mélodie
// ─────────────────────────────────────────────
struct MelodyNote {
    uint32_t freq_hz; ///< Fréquence (0 = silence)
    uint32_t dur_ms; ///< Durée totale de la note
    uint32_t gap_ms; ///< Silence après la note (défaut recommandé : dur_ms/8)
};

/** Mélodie : tableau de notes + longueur */
struct Melody {
    const MelodyNote *notes;
    uint8_t count;
    uint8_t repeat; ///< 0 = une seule fois
};

// ─────────────────────────────────────────────
//  Mélodies prédéfinies
// ─────────────────────────────────────────────
extern const Melody MELODY_STARTUP;
extern const Melody MELODY_SUCCESS;
extern const Melody MELODY_ERROR;
extern const Melody MELODY_ALARM;
extern const Melody MELODY_CLICK;
extern const Melody MELODY_DOUBLE_BEEP;

// ─────────────────────────────────────────────
//  Classe Buzzer
// ─────────────────────────────────────────────
class Buzzer {
public:
    /**
     * @brief Constructeur
     * @param pin       GPIO du buzzer
     * @param channel   Canal LEDC [0..7] (doit être différent des autres LEDC)
     * @param passive   true = buzzer passif (génère la fréquence via LEDC)
     *                  false = buzzer actif (on/off uniquement)
     */
    explicit Buzzer(uint8_t pin, uint8_t channel = 1, bool passive = true);

    /** Initialise le GPIO / LEDC. Appeler dans setup(). */
    void begin();

    // ── API tone/noTone (compatible Arduino) ─
    /**
     * @brief Joue une tonalité continue
     * @param freq_hz Fréquence en Hz (ignorée si buzzer actif)
     */
    void tone(uint32_t freq_hz = Note::BEEP_STD);

    /** Arrête la tonalité */
    void noTone();

    // ── Beep non-bloquant ────────────────────
    /**
     * @brief Joue un beep non-bloquant
     * @param freq_hz  Fréquence Hz
     * @param dur_ms   Durée en ms
     */
    void beep(uint32_t freq_hz = Note::BEEP_STD, uint32_t dur_ms = 100);

    /**
     * @brief Joue N beeps non-bloquants
     * @param times    Nombre de beeps
     * @param freq_hz  Fréquence
     * @param dur_ms   Durée de chaque beep
     * @param gap_ms   Silence entre les beeps
     */
    void beepN(uint8_t times, uint32_t freq_hz = Note::BEEP_STD, uint32_t dur_ms = 100, uint32_t gap_ms = 100);

    // ── Mélodies ─────────────────────────────
    void play(const Melody &melody);

    void playStartup() { play(MELODY_STARTUP); }

    void playSuccess() { play(MELODY_SUCCESS); }

    void playError() { play(MELODY_ERROR); }

    void playAlarm(); ///< Alarme continue bitonale (utiliser stop() pour arrêter)

    void click() { play(MELODY_CLICK); }

    void doubleBeep() { play(MELODY_DOUBLE_BEEP); }

    /** Arrête tout (mélodie, beep, alarme) */
    void stop();

    /**
     * @brief À appeler dans loop() pour les modes non-bloquants
     * @return true si une transition s'est produite
     */
    bool update();

    // ── Volume (buzzer passif) ───────────────
    /**
     * @brief Définit le rapport cyclique PWM (volume apparent)
     * @param duty 0–100 (%)
     */
    void setVolume(uint8_t duty_pct);

    // ── État ────────────────────────────────
    bool isPlaying() const { return _playing; }

    bool isActive() const { return _active; }

    uint32_t currentFreq() const { return _currentFreq; }

private:
    uint8_t _pin;
    uint8_t _channel;
    bool _passive;
    uint8_t _volume{50}; ///< duty % (50 = onde carrée pur)
    bool _active{false};
    bool _playing{false};
    uint32_t _currentFreq{0};

    // Séquence courante
    const Melody *_melody{nullptr};
    uint8_t _noteIndex{0};
    uint8_t _repeatCount{0};
    uint32_t _lastTime{0};
    bool _inGap{false};

    // Beep simple / beepN
    bool _simpleBeep{false};
    uint32_t _beepDur{0};
    uint32_t _beepGap{0};
    uint8_t _beepTimes{0};
    uint8_t _beepCount{0};
    uint32_t _beepFreq{0};

    // Alarme bitonale continue
    bool _alarm{false};
    uint32_t _alarmPhase{0};
    uint32_t _alarmLastTime{0};

    void _startFreq(uint32_t freq_hz);
    void _stopFreq();
    uint32_t _dutyValue() const;
};
