#include <Arduino.h>

#include "LowWare/BLE/BluetoothManager.h"
#include "MiddleWare/IHM.h"
#include "MiddleWare/Sensors.h"

Sensors sensors;
IHM ihm;
BLEManager bleManager;

float voltage;
float current;
float tempPcb;
float tempAmb1;
float tempAmb2;

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("BOOT OK");

    if (!sensors.init()) return;
    (void) ihm.init();
}

void loop() {
    sensors.checkSensors(voltage, current, tempPcb, tempAmb1, tempAmb2);
    ihm.update(voltage, current, tempPcb, tempAmb1, tempAmb2);
    delay(500);
}
