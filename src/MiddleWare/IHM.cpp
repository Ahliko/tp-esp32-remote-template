//
// Created by Ahliko on 01/07/2026.
//

#include "IHM.h"

#include "LowWare/BLE/BluetoothManager.h"

IHM::IHM() {
    m_buzzer = new Buzzer(BUZZER_PIN);
    m_green_led = new LED(LED_GREEN_PIN);
    m_red_led = new LED(LED_RED_PIN);
    m_bleManager = new BLEManager();
    m_config = LimitConfig();
}

bool IHM::init() {
    m_buzzer->init();
    m_green_led->init();
    m_red_led->init();
    m_bleManager->init(BLE_DEVICE_NAME);
    return true;
}

void IHM::update(float &voltage, float &current, float &temp) {
    AppConfig cfg = m_bleManager->getConfig();
    if (cfg.isUpdated) {
        m_config.temp_limit_high = m_bleManager->getConfig().config.temp_limit_high;
        m_bleManager->clearUpdateFlag();
    }
    checkValues(voltage, current, temp);
    m_bleManager->updateTelemetry(current, temp, temp, 0);
}

void IHM::checkValues(float &voltage, float &current, float &temp) {
    if (m_config.voltage_limit_low > voltage || voltage > m_config.voltage_limit_high) {
        Serial.println("ALERT on Voltage");
        displayAlert();
        return;
    }

    if (m_config.current_limit_low > current || current > m_config.current_limit_high) {
        Serial.println("ALERT on Current");
        displayAlert();
        return;
    }

    if (m_config.temp_limit_low > temp || temp > m_config.temp_limit_high) {
        Serial.println("ALERT on Temp");
        Serial.printf("low : %f, high : %f, temp : %f\n", m_config.temp_limit_low, m_config.temp_limit_high, temp);
        displayAlert();
        return;
    }
    displayAndSendValues();
}

void IHM::displayAndSendValues() {
    m_isAlert = false;
    ledGreenOn();
    ledRedOff();
    m_buzzer->stop();
}

void IHM::displayAlert() {
    if (!m_isAlert) {
        m_buzzer->playAlarm();
        m_isAlert = true;
    }
    ledGreenOff();
    ledRedOn();
}

void IHM::ledGreenOn() const { m_green_led->on(); }

void IHM::ledRedOn() const { m_red_led->on(); }

void IHM::ledGreenOff() const { m_green_led->off(); }

void IHM::ledRedOff() const { m_red_led->off(); }
