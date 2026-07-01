#pragma once
/**
 * @file LED.h
 * @brief Driver C++ générique pour LEDs sur ESP32
 *        Supporte : on/off, toggle, blink (bloquant & non-bloquant),
 *                   fade PWM (LEDC), SOS, patterns personnalisés
 *
 * Deux modes de fonctionnement :
 *   - Mode DIGITAL  : GPIO simple en sortie
 *   - Mode PWM      : canal LEDC pour intensité variable / fade
 *
 * Usage non-bloquant : appeler update() dans loop() régulièrement.
 */

#include <Arduino.h>
#include <cstdint>
#include <functional>

// ─────────────────────────────────────────────
//  Configuration LEDC
// ─────────────────────────────────────────────
static constexpr uint8_t LED_PWM_BITS = 10; ///< Résolution PWM (0–1023)
static constexpr uint32_t LED_PWM_FREQ_HZ = 5000; ///< Fréquence LEDC
static constexpr uint8_t LED_MAX_CHANNEL = 8; ///< Canaux LEDC disponibles (ESP32)
static constexpr uint32_t LED_PWM_MAX_DUTY = (1u << LED_PWM_BITS) - 1;

// ─────────────────────────────────────────────
//  Types
// ─────────────────────────────────────────────

enum class LEDMode : uint8_t {
    DIGITAL, ///< GPIO simple
    PWM, ///< LEDC (fade, intensité)
};

/** Un pas de pattern : durée ON (ms) + durée OFF (ms) */
struct LEDStep {
    uint32_t on_ms;
    uint32_t off_ms;
};

/** Pattern complet : tableau de steps + nombre de répétitions (0 = infini) */
struct LEDPattern {
    const LEDStep *steps;
    uint8_t count;
    uint8_t repeat; ///< 0 = infini
};

// ─────────────────────────────────────────────
//  Patterns prédéfinis (déclarés dans .cpp)
// ─────────────────────────────────────────────
extern const LEDPattern LED_PATTERN_SOS;
extern const LEDPattern LED_PATTERN_HEARTBEAT;
extern const LEDPattern LED_PATTERN_SLOW_BLINK;
extern const LEDPattern LED_PATTERN_FAST_BLINK;

// ─────────────────────────────────────────────
//  Classe LED
// ─────────────────────────────────────────────
class LED {
public:
    /**
     * @brief Constructeur
     * @param pin     GPIO de la LED
     * @param mode    DIGITAL ou PWM
     * @param channel Canal LEDC (ignoré si DIGITAL) [0..7]
     * @param inverted true si LED câblée en active-low
     */
    explicit LED(uint8_t pin, LEDMode mode = LEDMode::DIGITAL, uint8_t channel = 0, bool inverted = false);

    /** Initialise le GPIO / LEDC. Appeler après Serial.begin() si besoin de logs. */
    void begin();

    // ── Contrôle de base ────────────────────
    void on();
    void off();
    void toggle();

    /** PWM : intensité 0.0 → 1.0 (ignoré en mode DIGITAL) */
    void setIntensity(float intensity);

    /** Fade vers intensité cible en `duration_ms` ms (mode PWM uniquement) */
    void fadeTo(float targetIntensity, uint32_t duration_ms);

    // ── Blink bloquant ──────────────────────
    /** Blink n fois de manière bloquante (simple, utiliser en setup ou tests) */
    void blinkBlocking(uint8_t times, uint32_t on_ms = 200, uint32_t off_ms = 200);

    // ── Mode non-bloquant ───────────────────
    /**
     * @brief Lance un blink non-bloquant
     * @param on_ms   Durée ON (ms)
     * @param off_ms  Durée OFF (ms)
     * @param times   Nombre de clignotements (0 = infini)
     */
    void startBlink(uint32_t on_ms = 500, uint32_t off_ms = 500, uint8_t times = 0);

    /** Lance un pattern non-bloquant */
    void startPattern(const LEDPattern &pattern);

    /** Lance le pattern SOS (3 courts + 3 longs + 3 courts) */
    void startSOS();

    /** Lance le pattern battement de cœur */
    void startHeartbeat();

    /** Arrête blink / pattern en cours */
    void stop();

    /**
     * @brief À appeler dans loop() pour blink/pattern non-bloquants
     * @return true si une transition s'est produite
     */
    bool update();

    // ── État ────────────────────────────────
    bool isOn() const { return _state; }

    bool isActive() const { return _running; }

    uint8_t getPin() const { return _pin; }

    float getIntensity() const { return _intensity; }

    /** Callback appelé à chaque changement d'état (optionnel) */
    void onStateChange(std::function<void(bool)> cb) { _callback = cb; }

private:
    uint8_t _pin;
    LEDMode _mode;
    uint8_t _channel;
    bool _inverted;
    bool _state{false};
    float _intensity{1.0f};

    // Blink / pattern
    bool _running{false};
    uint32_t _lastTime{0};
    bool _inOnPhase{false};

    // Pattern
    const LEDPattern *_pattern{nullptr};
    uint8_t _stepIndex{0};
    uint8_t _repeatCount{0};

    // Simple blink (si pas de pattern)
    uint32_t _blinkOnMs{500};
    uint32_t _blinkOffMs{500};
    uint8_t _blinkTimes{0};
    uint8_t _blinkCount{0};

    // Fade
    bool _fading{false};
    float _fadeStart{0.0f};
    float _fadeTarget{0.0f};
    uint32_t _fadeDuration{0};
    uint32_t _fadeStartTime{0};

    std::function<void(bool)> _callback;

    void _writeState(bool on);
    void _writeDuty(float intensity);
    uint32_t _currentOnMs() const;
    uint32_t _currentOffMs() const;
    void _notifyChange(bool newState);
};
