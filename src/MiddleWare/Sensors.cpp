//
// Created by Ahliko on 01/07/2026.
//

#include "Sensors.h"

#include "Config/config.h"

Sensors::Sensors() : m_tmp126low(TMP126Low(CSPin, SpiFreq)) {
    m_ina237 = new INA237();
    m_tmp126 = new TMP126(m_tmp126low);
    m_thermistor1 = new Thermistor(Thermistor1_Pin, SeriesResistor, NominalResistor, BetaValue, true);
    m_thermistor2 = new Thermistor(Thermistor2_Pin, SeriesResistor, NominalResistor, BetaValue, true);
}

bool Sensors::init() const {
    m_ina237->init();
    if (!m_tmp126->init()) {
        Serial.println("TMP init failed");
        return false;
    };

    m_thermistor1->init();
    m_thermistor2->init();
    return true;
}

void Sensors::checkSensors(float &voltage, float &current, float &tempPcb, float &tempAmb1, float &tempAmb2) {
    voltage = m_ina237->readBusVoltage();
    current = m_ina237->readCurrent();
    tempPcb = m_tmp126->readTemperature();
    tempAmb1 = m_thermistor1->readTemperature();
    tempAmb2 = m_thermistor2->readTemperature();
    Serial.printf("amb1 : %f\t amb2 : %f\n ", tempAmb1, tempAmb2);
}
