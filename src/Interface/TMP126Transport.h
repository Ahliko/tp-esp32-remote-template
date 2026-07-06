#pragma once
#include <cstdint>

class TMP126Transport {
public:
    virtual ~TMP126Transport() = default;

    /**
     * @brief Initialise le bus matériel (si nécessaire)
     * @return true si l'init est OK
     */
    virtual bool initBus() const = 0;

    /**
     * @brief Lit un registre 16 bits depuis le capteur
     */
    virtual uint16_t readRegRaw(uint16_t cmdWord) const = 0;

    /**
     * @brief Écrit une valeur 16 bits dans un registre
     */
    virtual void writeRegRaw(uint16_t cmdWord, uint16_t dataWord) const = 0;

    /**
     * @brief Utilitaires de temps pour isoler complètement Arduino de la logique
     */
    virtual uint32_t getMillis() const = 0;
    virtual void delayUs(uint32_t us) const = 0;
};