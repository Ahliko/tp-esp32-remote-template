#include <gtest/gtest.h>
#include "BLEManagerTest.h"

TEST(IHMTest, CheckTelemetryIsSentToBLE) {
    BLEManagerTest mockBle;
    mockBle.updateTelemetry(12.5f, 40.0f, 25.0f, 25.0f, 0);

    EXPECT_FLOAT_EQ(12.5f, mockBle.lastCurrent);
    EXPECT_FLOAT_EQ(40.0f, mockBle.lastPcbTemp);
}

TEST(IHMTest, CheckConfigUpdate) {
    BLEManagerTest mockBle;
    
    mockBle.simulateClientWriteConfig(15.0f, 60.0f);
    
    AppConfig cfg = mockBle.getConfig();
    EXPECT_TRUE(cfg.isUpdated);
    EXPECT_FLOAT_EQ(15.0f, cfg.config.current_limit_high);
    
    mockBle.clearUpdateFlag();
    EXPECT_FALSE(mockBle.getConfig().isUpdated);
}