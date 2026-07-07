//
// Created by Ahliko on 01/07/2026.
//

#include "IHM.h"

#include "LowWare/BLE/BLEManagerLow.h"

IHM::IHM() {
    m_buzzer = new Buzzer(BUZZER_PIN);
    m_green_led = new LED(LED_GREEN_PIN);
    m_red_led = new LED(LED_RED_PIN);
    m_bleManager = new BLEManagerLow();
    m_config = LimitConfig();
}

bool IHM::init() {
    m_buzzer->init();
    m_green_led->init();
    m_red_led->init();
    m_bleManager->init(BLE_DEVICE_NAME);
    return true;
}

void IHM::update(float &voltage, float &current, float &tempPcb, float &tempAmb1, float &tempAmb2) {
    AppConfig cfg = m_bleManager->getConfig();
    if (cfg.isUpdated) {
        m_config = m_bleManager->getConfig().config;
        m_bleManager->clearUpdateFlag();
    }
    checkValues(voltage, current, tempPcb, tempAmb1, tempAmb2);
    m_bleManager->updateTelemetry(current, tempPcb, tempAmb1, tempAmb2, m_isAlert);
}

void IHM::checkValues(float &voltage, float &current, float &tempPcb, float &tempAmb1, float &tempAmb2) {
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

    if (m_config.temp_pcb_limit_low > tempPcb || tempPcb > m_config.temp_pcb_limit_high) {
        Serial.println("ALERT on TempPCB");
        Serial.printf("low : %f, high : %f, temp : %f\n", m_config.temp_pcb_limit_low, m_config.temp_pcb_limit_high, tempPcb);
        displayAlert();
        return;
    }

    if (m_config.temp_amb_limit_low > tempAmb1 || tempAmb1 > m_config.temp_amb_limit_high) {
        Serial.println("ALERT on TempAMB1");
        Serial.printf("low : %f, high : %f, temp : %f\n", m_config.temp_pcb_limit_low, m_config.temp_pcb_limit_high, tempAmb1);
        displayAlert();
        return;
    }

    if (m_config.temp_amb_limit_low > tempAmb2 || tempAmb2 > m_config.temp_amb_limit_high) {
        Serial.println("ALERT on TempAMB2");
        Serial.printf("low : %f, high : %f, temp : %f\n", m_config.temp_pcb_limit_low, m_config.temp_pcb_limit_high, tempAmb2);
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
