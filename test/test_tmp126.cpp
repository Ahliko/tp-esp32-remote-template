//
// Created by Ahliko on 06/07/2026.
//
#include <gtest/gtest.h>
#include <cmath>
#include "../src/LowWare/Capteurs/TMP/TMP126.h"

// Si vous avez placé TMP126LowTest.h dans le même dossier de test :
#include "LowWareTest/TMP126LowTest.h"

// ─────────────────────────────────────────────────────────
// 1. Tests des fonctions statiques de conversion
// ─────────────────────────────────────────────────────────

// Test de la conversion Raw vers Température
TEST(TMP126Test, RawToTempConversion) {
    // 0°C devrait être 0x0000
    EXPECT_NEAR(0.0f, TMP126::rawToTemp(0x0000), 0.001f);

    // +25°C
    EXPECT_NEAR(25.0f, TMP126::rawToTemp(TMP126::tempToRaw(25.0f)), 0.001f);

    // Température négative (-25°C)
    EXPECT_NEAR(-25.0f, TMP126::rawToTemp(TMP126::tempToRaw(-25.0f)), 0.001f);
}

// Test de la construction des commandes SPI
TEST(TMP126Test, BuildCmdFormat) {
    // Read TEMP_RESULT (0x00) sans AutoInc
    uint16_t cmdRead = TMP126::buildCmd(TMP126Reg::TEMP_RESULT, true, false);
    EXPECT_EQ(cmdRead, 0x0100);

    // Write CONFIG (0x03) sans AutoInc
    uint16_t cmdWrite = TMP126::buildCmd(TMP126Reg::CONFIG, false, false);
    EXPECT_EQ(cmdWrite, 0x0003);
}


// ─────────────────────────────────────────────────────────
// 2. Tests de la logique métier avec l'injection du Mock
// ─────────────────────────────────────────────────────────

TEST(TMP126LogicTest, InitializationSuccess) {
    TMP126LowTest mockTransport; // Fausse interface SPI
    TMP126 sensor(mockTransport); // Injection dans la classe métier

    // Par défaut le mock a 0x1126 au registre DEVICE_ID (0x0C)
    EXPECT_TRUE(sensor.init());
}

TEST(TMP126LogicTest, ReadTemperatureSequence) {
    TMP126LowTest mockTransport;
    TMP126 sensor(mockTransport);

    // On simule que la sonde physique répond "42.5 °C"
    mockTransport.simulateTemperatureRaw(TMP126::tempToRaw(42.5f));

    // La logique métier (sensor) doit envoyer la bonne commande et convertir la réponse
    EXPECT_NEAR(42.5f, sensor.readTemperature(), 0.001f);
}

TEST(TMP126LogicTest, OneShotBlockingTimeout) {
    TMP126LowTest mockTransport;
    TMP126 sensor(mockTransport);

    // On ne met pas le flag "Data_Ready" à true dans le mock,
    // donc ça doit timeout après 20ms et renvoyer NAN
    float result = sensor.readOneShotBlocking(20);
    EXPECT_TRUE(std::isnan(result));
}

// ─────────────────────────────────────────────────────────
// Point d'entrée obligatoire pour l'environnement natif GTest
// ─────────────────────────────────────────────────────────
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);

    if (RUN_ALL_TESTS()) {
        // PlatformIO gère le retour
    }

    return 0;
} // <-- Il manquait cette accolade dans votre code !