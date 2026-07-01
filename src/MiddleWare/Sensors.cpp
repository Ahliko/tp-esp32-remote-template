//
// Created by Ahliko on 01/07/2026.
//

#include "Sensors.h"

Sensors::Sensors() {
    m_ina237 = new INA237();
    m_tmp126 = new TMP126();
}

bool Sensors::init() const {
    m_ina237->init();
    if (!m_tmp126->init()) {
        Serial.println("TMP init failed");
        return false;
    };
    return true;
}

void Sensors::checkSensors(float &voltage, float &current, float &temp) {
    voltage = m_ina237->readBusVoltage();
    current = m_ina237->readCurrent();
    temp = m_tmp126->readTemperature();
    Serial.println(voltage);
    Serial.println(current);
    Serial.println(temp);
}
