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
}

bool IHM::init() const {
    m_buzzer->init();
    m_green_led->init();
    m_red_led->init();
    m_bleManager->init(BLE_DEVICE_NAME);
    return true;
}

void IHM::checkValues(float &voltage, float &current, float &temp) {
    if (VOLTAGE_LIMIT_LOW > voltage || voltage > VOLTAGE_LIMIT_HIGH) {
        Serial.println("ALERT on Voltage");
        displayAlert();
        return;
    }

    if (CURRENT_LIMIT_LOW > current || current > CURRENT_LIMIT_HIGH) {
        Serial.println("ALERT on Current");
        displayAlert();
        return;
    }

    if (TEMP_LIMIT_LOW > temp || temp > TEMP_LIMIT_HIGH) {
        Serial.println("ALERT on Temp");
        displayAlert();
        return;
    }
    Serial.println("NO Alert");
    displayAndSendValues();
    AppConfig cfg = m_bleManager->getConfig();
    if (cfg.isUpdated) {
        m_bleManager->clearUpdateFlag();
    }
}

void IHM::displayAndSendValues() const {
    ledGreenOn();
    ledRedOff();
    m_buzzer->stop();
}

void IHM::displayAlert() const {
    m_buzzer->playAlarm();
    ledGreenOff();
    ledRedOn();
}

void IHM::ledGreenOn() const { m_green_led->on(); }

void IHM::ledRedOn() const { m_red_led->on(); }

void IHM::ledGreenOff() const { m_green_led->off(); }

void IHM::ledRedOff() const { m_red_led->off(); }
