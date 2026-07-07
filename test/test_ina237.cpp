#include <gtest/gtest.h>

#include "../src/Interface/INA237Transport.h"
#include "../src/LowWare/Capteurs/INA/INA237.h" // Modifiez selon votre dossier
#include "LowWareTest/INA237LowTest.h"

TEST(INA237LogicTest, InitializationSuccess) {
    INA237LowTest mockTransport;
    INA237 sensor(mockTransport, 0.01f, 10.0f);

    EXPECT_TRUE(sensor.init(ADCRange::RANGE_163_84mV));

    EXPECT_EQ((mockTransport.registers[INA237Reg::CONFIG] & (1 << 4)), 0);
}

TEST(INA237LogicTest, ReadBusVoltage) {
    INA237LowTest mockTransport;
    INA237 sensor(mockTransport, 0.01f, 10.0f);

    mockTransport.simulateBusVoltageRaw(7680);

    EXPECT_NEAR(24.0f, sensor.readBusVoltage(), 0.01f);
}

TEST(INA237LogicTest, CalibrateCalculation) {
    float currentLSB = 10.0f / 32768.0f;
    float shuntOhms = 0.01f;

    uint16_t expected_cal = static_cast<uint16_t>(13107.2e6f * currentLSB * shuntOhms);
    uint16_t calculated_cal = INA237::calcShuntCal(currentLSB, shuntOhms, ADCRange::RANGE_163_84mV);

    EXPECT_EQ(expected_cal, calculated_cal);
}

TEST(INA237LogicTest, ReadCurrent_Zero) {
    INA237LowTest mockTransport;
    INA237 sensor(mockTransport, 0.01f, 10.0f); // Max 10A

    // Il faut initialiser le capteur pour que _currentLSB soit calculé (10A / 32768)
    sensor.init(ADCRange::RANGE_163_84mV);

    // On simule un registre de courant à 0
    mockTransport.simulateCurrentRaw(0x0000);

    EXPECT_FLOAT_EQ(0.0f, sensor.readCurrent());
}

TEST(INA237LogicTest, ReadCurrent_PositiveCurrent) {
    INA237LowTest mockTransport;
    INA237 sensor(mockTransport, 0.01f, 10.0f);
    sensor.init(ADCRange::RANGE_163_84mV);

    // currentLSB = 10 / 32768 = 0.00030517578
    // Si on veut simuler environ 5 Ampères : 5 / currentLSB = 16384 (0x4000)
    mockTransport.simulateCurrentRaw(0x4000);

    // Tolérance de 0.001A à cause des arrondis flottants
    EXPECT_NEAR(5.0f, sensor.readCurrent(), 0.001f);
}

TEST(INA237LogicTest, ReadCurrent_MaxPositiveCurrent) {
    INA237LowTest mockTransport;
    INA237 sensor(mockTransport, 0.01f, 10.0f);
    sensor.init(ADCRange::RANGE_163_84mV);

    // La valeur positive maximale d'un int16_t est 32767 (0x7FFF)
    // Ce qui devrait correspondre exactement au maxCurrentA (10A) moins 1 LSB
    mockTransport.simulateCurrentRaw(0x7FFF);

    float expected_current = 10.0f - sensor.getCurrentLSB();
    EXPECT_NEAR(expected_current, sensor.readCurrent(), 0.001f);
}

TEST(INA237LogicTest, ReadCurrent_NegativeCurrent) {
    INA237LowTest mockTransport;
    INA237 sensor(mockTransport, 0.01f, 10.0f);
    sensor.init(ADCRange::RANGE_163_84mV);

    // L'INA237 stocke le courant en complément à deux (int16_t).
    // -5 Ampères correspond à -16384 (0xC000 en hexadécimal 16-bit)
    mockTransport.simulateCurrentRaw(0xC000);

    EXPECT_NEAR(-5.0f, sensor.readCurrent(), 0.001f);
}

TEST(INA237LogicTest, ReadCurrent_MaxNegativeCurrent) {
    INA237LowTest mockTransport;
    INA237 sensor(mockTransport, 0.01f, 10.0f);
    sensor.init(ADCRange::RANGE_163_84mV);

    // La valeur négative maximale d'un int16_t est -32768 (0x8000)
    // Ce qui devrait correspondre exactement à -maxCurrentA (-10A)
    mockTransport.simulateCurrentRaw(0x8000);

    EXPECT_NEAR(-10.0f, sensor.readCurrent(), 0.001f);
}