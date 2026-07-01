//
// Created by Ahliko on 01/07/2026.
//

#ifndef TP_ESP32_REMOTE_TEMPLATE_CONFIG_H
#define TP_ESP32_REMOTE_TEMPLATE_CONFIG_H

#define BUZZER_PIN 13
#define LED_RED_PIN 15
#define LED_GREEN_PIN 14

#define CSPin 5
#define SpiFreq 1000000UL

#define INAADDR 0x40
#define ShuntOhms 500.0
#define MaxCurrentA 30.0

#define BLE_DEVICE_NAME "ESP32"

#define VOLTAGE_LIMIT_LOW 0
#define VOLTAGE_LIMIT_HIGH 30
#define CURRENT_LIMIT_LOW 0
#define CURRENT_LIMIT_HIGH 15
#define TEMP_LIMIT_LOW 0
#define TEMP_LIMIT_HIGH 75

struct LimitConfig {
    float voltage_limit_low = VOLTAGE_LIMIT_LOW;
    float voltage_limit_high = VOLTAGE_LIMIT_HIGH;
    float current_limit_low = CURRENT_LIMIT_LOW;
    float current_limit_high = CURRENT_LIMIT_HIGH;
    float temp_limit_low = TEMP_LIMIT_LOW;
    float temp_limit_high = TEMP_LIMIT_HIGH;
};


#endif // TP_ESP32_REMOTE_TEMPLATE_CONFIG_H
