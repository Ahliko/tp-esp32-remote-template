#include <gtest/gtest.h>
#include <cmath>
#include "../src/LowWare/Capteurs/TMP/TMP126.h"

#include "LowWareTest/TMP126LowTest.h"
TEST(TMP126Test, RawToTempConversion) {
    // 0°C devrait être 0x0000
    EXPECT_NEAR(0.0f, TMP126::rawToTemp(0x0000), 0.001f);

    // +25°C
    EXPECT_NEAR(25.0f, TMP126::rawToTemp(TMP126::tempToRaw(25.0f)), 0.001f);

    // Température négative (-25°C)
    EXPECT_NEAR(-25.0f, TMP126::rawToTemp(TMP126::tempToRaw(-25.0f)), 0.001f);
}
TEST(TMP126Test, BuildCmdFormat) {
    // Read TEMP_RESULT (0x00) sans AutoInc
    uint16_t cmdRead = TMP126::buildCmd(TMP126Reg::TEMP_RESULT, true, false);
    EXPECT_EQ(cmdRead, 0x0100);

    // Write CONFIG (0x03) sans AutoInc
    uint16_t cmdWrite = TMP126::buildCmd(TMP126Reg::CONFIG, false, false);
    EXPECT_EQ(cmdWrite, 0x0003);
}

TEST(TMP126LogicTest, InitializationSuccess) {
    TMP126LowTest mockTransport;
    TMP126 sensor(mockTransport);
    EXPECT_TRUE(sensor.init());
}

TEST(TMP126LogicTest, ReadTemperatureSequence) {
    TMP126LowTest mockTransport;
    TMP126 sensor(mockTransport);

    mockTransport.simulateTemperatureRaw(TMP126::tempToRaw(42.5f));
    EXPECT_NEAR(42.5f, sensor.readTemperature(), 0.001f);
}

TEST(TMP126LogicTest, OneShotBlockingTimeout) {
    TMP126LowTest mockTransport;
    TMP126 sensor(mockTransport);
    float result = sensor.readOneShotBlocking(20);
    EXPECT_TRUE(std::isnan(result));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);

    if (RUN_ALL_TESTS()) {
    }

    return 0;
}