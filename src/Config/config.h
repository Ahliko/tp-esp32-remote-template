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

#define VOLTAGE_LIMIT_LOW 2
#define VOLTAGE_LIMIT_HIGH 30
#define CURRENT_LIMIT_LOW 2
#define CURRENT_LIMIT_HIGH 35
#define TEMP_LIMIT_LOW 2
#define TEMP_LIMIT_HIGH 100


#endif // TP_ESP32_REMOTE_TEMPLATE_CONFIG_H
