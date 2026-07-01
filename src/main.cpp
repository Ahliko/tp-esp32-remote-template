#include <Arduino.h>

#include "Capteurs/INA237.h"

INA237 ina237(INA237Address::GND_GND);

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("BOOT OK");
    if (!ina237.begin(0.010f, 20.0f, ADCRange::RANGE_163_84mV)) {
        Serial.println("[INA237] Erreur d'initialisation ! Vérifiez le câblage.");
        while (true)
            delay(1000);
    }

    Serial.printf("[INA237] Manufacturer ID : 0x%04X\n", ina237.readManufacturerId());
    Serial.printf("[INA237] Device ID       : 0x%04X\n", ina237.readDeviceId());
    Serial.printf("[INA237] CurrentLSB      : %.3f µA\n", ina237.getCurrentLSB() * 1e6f);
}

void loop() {
    uint32_t t0 = millis();
    while (!ina237.isConversionReady() && (millis() - t0 < 100)) {
        delayMicroseconds(100);
    }

    float vbus = ina237.readBusVoltage();
    float vshunt = ina237.readShuntVoltage();
    float current = ina237.readCurrent();
    float power = ina237.readPower();
    float temp = ina237.readTemperature();

    DiagAlert diag = ina237.readDiagAlert();

    Serial.printf("Vbus=%.3fV  Vshunt=%.6fV  I=%.4fA  P=%.4fW  T=%.2f°C", vbus, vshunt, current, power, temp);

    if (diag.busol)
        Serial.print("  [ALERTE: Vbus > limite]");
    if (diag.mathof)
        Serial.print("  [OVERFLOW MATH]");
    if (!diag.memstat)
        Serial.print("  [ERREUR MEMOIRE]");
    Serial.println();

    delay(500);
}
