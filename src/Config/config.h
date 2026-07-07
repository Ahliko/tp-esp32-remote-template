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

#define Thermistor1_Pin 26
#define Thermistor2_Pin 25
#define SeriesResistor 10000.0
#define NominalResistor 10000.0
#define BetaValue 3630

#define BLE_DEVICE_NAME "ESP32"
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHAR_CURRENT_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define CHAR_PCB_TEMP_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a9"
#define CHAR_AMB1_TEMP_UUID "beb5483e-36e1-4688-b7f5-ea07361b26aa"
#define CHAR_AMB2_TEMP_UUID "beb5483e-36e1-4688-b7f5-ea07361b26ab"
#define CHAR_ALARM_STAT_UUID "beb5483e-36e1-4688-b7f5-ea07361b26ac"
#define CHAR_LOGS_UUID "beb5483e-36e1-4688-b7f5-ea07361b26ad"
#define CHAR_CFG_MAX_CURRENT_UUID "beb5483e-36e1-4688-b7f5-ea07361b26ae"
#define CHAR_CFG_MAX_TEMP_PCB_UUID "beb5483e-36e1-4688-b7f5-ea07361b26af"
#define CHAR_CFG_MAX_TEMP_AMB_UUID "beb5483e-36e1-4688-b7f5-ea07361b26b0"

#define VOLTAGE_LIMIT_LOW 0
#define VOLTAGE_LIMIT_HIGH 30
#define CURRENT_LIMIT_LOW 0
#define CURRENT_LIMIT_HIGH 15
#define TEMP_PCB_LIMIT_LOW 0
#define TEMP_PCB_LIMIT_HIGH 75
#define TEMP_AMB_LIMIT_LOW 0
#define TEMP_AMB_LIMIT_HIGH 75


struct LimitConfig {
    float voltage_limit_low = VOLTAGE_LIMIT_LOW;
    float voltage_limit_high = VOLTAGE_LIMIT_HIGH;
    float current_limit_low = CURRENT_LIMIT_LOW;
    float current_limit_high = CURRENT_LIMIT_HIGH;
    float temp_pcb_limit_low = TEMP_PCB_LIMIT_LOW;
    float temp_pcb_limit_high = TEMP_PCB_LIMIT_HIGH;
    float temp_amb_limit_low = TEMP_AMB_LIMIT_LOW;
    float temp_amb_limit_high = TEMP_AMB_LIMIT_HIGH;
};


#endif // TP_ESP32_REMOTE_TEMPLATE_CONFIG_H
