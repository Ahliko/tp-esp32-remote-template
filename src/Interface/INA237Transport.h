#pragma once
#include <cstdint>

class INA237Transport {
public:
    virtual ~INA237Transport() = default;

    /**
     * @brief Initialise le bus I2C matériel
     * @return true si l'init est OK
     */
    virtual bool initBus() const = 0;

    /**
     * @brief Lit un registre 16 bits à une adresse spécifique
     * @param reg Adresse du registre (0x00 à 0x3F)
     * @return Valeur du registre, ou 0xFFFF en cas d'erreur
     */
    virtual uint16_t readReg(uint8_t reg) const = 0;

    /**
     * @brief Écrit une valeur 16 bits à une adresse spécifique
     * @param reg Adresse du registre
     * @param value Valeur à écrire
     */
    virtual void writeReg(uint8_t reg, uint16_t value) const = 0;

    /**
     * @brief Attente non-bloquante pour la stabilisation et les tests
     */
    virtual void delayMs(uint32_t ms) const = 0;
};
