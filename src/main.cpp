#include <Arduino.h>

#include "LowWare/BLE/BluetoothManager.h"
#include "MiddleWare/IHM.h"
#include "MiddleWare/Sensors.h"

Sensors sensors;
IHM ihm;
BLEManager bleManager;

float voltage;
float current;
float temp;

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("BOOT OK");

    if (!sensors.init()) return;
    (void) ihm.init();
}

void loop() {
    sensors.checkSensors(voltage, current, temp);
    ihm.checkValues(voltage, current, temp);
    delay(500);
}
