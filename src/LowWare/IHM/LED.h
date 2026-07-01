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

/** Un pas de pattern : durée ON (ms) + durée OFF (ms) */
struct LEDStep {
    uint32_t on_ms;
    uint32_t off_ms;
};


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
    explicit LED(uint8_t pin, bool inverted = false);

    /** Initialise le GPIO / LEDC. Appeler après Serial.begin() si besoin de logs. */
    void init();

    // ── Contrôle de base ────────────────────
    void on();
    void off();
    void toggle();



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


private:
    uint8_t _pin;
    bool _inverted;
    bool _state{false};
    float _intensity{1.0f};

    // Blink / pattern
    bool _running{false};
    uint32_t _lastTime{0};
    bool _inOnPhase{false};

    // Simple blink (si pas de pattern)
    uint32_t _blinkOnMs{500};
    uint32_t _blinkOffMs{500};
    uint8_t _blinkTimes{0};
    uint8_t _blinkCount{0};

    void _writeState(bool on);
};
