#include <gtest/gtest.h>

#include "../src/Interface/INA237Transport.h"
#include "../src/LowWare/Capteurs/INA/INA237.h" // Modifiez selon votre dossier
#include "INA237LowTest.h"

TEST(INA237LogicTest, InitializationSuccess) {
    INA237LowTest mockTransport;
    // Shunt de 0.01 Ohms, max 10 Ampères
    INA237 sensor(mockTransport, 0.01f, 10.0f);

    EXPECT_TRUE(sensor.init(ADCRange::RANGE_163_84mV));

    // Vérifier que la config a bien été écrite dans le mock
    // Bit 4 (ADCRANGE) doit être à 0 pour RANGE_163_84mV
    EXPECT_EQ((mockTransport.registers[INA237Reg::CONFIG] & (1 << 4)), 0);
}

TEST(INA237LogicTest, ReadBusVoltage) {
    INA237LowTest mockTransport;
    INA237 sensor(mockTransport, 0.01f, 10.0f);

    // LSB = 3.125 mV. Donc pour 24V : 24 / 0.003125 = 7680 (0x1E00)
    mockTransport.simulateBusVoltageRaw(7680);

    EXPECT_NEAR(24.0f, sensor.readBusVoltage(), 0.01f);
}

TEST(INA237LogicTest, CalibrateCalculation) {
    // Test purement mathématique de la fonction statique
    float currentLSB = 10.0f / 32768.0f; // ~0.000305
    float shuntOhms = 0.01f;

    uint16_t expected_cal = static_cast<uint16_t>(13107.2e6f * currentLSB * shuntOhms);
    uint16_t calculated_cal = INA237::calcShuntCal(currentLSB, shuntOhms, ADCRange::RANGE_163_84mV);

    EXPECT_EQ(expected_cal, calculated_cal);
}
