//
// Created by Ahliko on 01/07/2026.
//

#ifndef TP_ESP32_REMOTE_TEMPLATE_IHM_H
#define TP_ESP32_REMOTE_TEMPLATE_IHM_H
#include "Config/config.h"
#include "LowWare/BLE/BluetoothManager.h"
#include "LowWare/IHM/Buzzer.h"
#include "LowWare/IHM/LED.h"

class IHM {
public:
    explicit IHM(); //BLEManager &ble, RAMManager &ram
    ~IHM() = default;

    bool init() const;
    void checkValues(float &voltage, float &current, float &temp);
private:
    void displayAndSendValues() const;
    void displayAlert() const;

    void ledGreenOn() const;
    void ledRedOn() const;
    void ledGreenOff() const;
    void ledRedOff() const;

    Buzzer *m_buzzer;
    LED *m_red_led;
    LED *m_green_led;
    BLEManager *m_bleManager;
};


#endif // TP_ESP32_REMOTE_TEMPLATE_IHM_H
