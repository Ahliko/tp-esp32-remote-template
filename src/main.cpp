#include <Arduino.h>

#include "LowWare/BLE/BluetoothManager.h"
#include "MiddleWare/IHM.h"
#include "MiddleWare/Sensors.h"

Sensors sensors;
IHM ihm;
BLEManager bleManager;
LimitConfig config;

float voltage;
float current;
float temp;

void setup() {
    config = LimitConfig();
    Serial.begin(115200);
    delay(1000);
    Serial.println("BOOT OK");

    if (!sensors.init()) return;
    (void) ihm.init(config);
}

void loop() {
    sensors.checkSensors(voltage, current, temp);
    ihm.checkValues(voltage, current, temp);
    delay(500);
}
