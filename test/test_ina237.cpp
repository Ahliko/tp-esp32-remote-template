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
