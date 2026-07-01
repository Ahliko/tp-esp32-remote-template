//
// Created by Ahliko on 01/07/2026.
//

#ifndef TP_ESP32_REMOTE_TEMPLATE_SENSORS_H
#define TP_ESP32_REMOTE_TEMPLATE_SENSORS_H
#include "LowWare/Capteurs/INA237.h"
#include "LowWare/Capteurs/TMP126.h"
#include "LowWare/Capteurs/Thermistor.h"

class Sensors {
public:
    explicit Sensors();
    ~Sensors() = default;

    bool init() const;
    void checkSensors(float &voltage, float &current, float &tempPcb, float &tempAmb1, float &tempAmb2);
private:
    INA237 *m_ina237;
    TMP126 *m_tmp126;
    Thermistor *m_thermistor1;
    Thermistor *m_thermistor2;
};


#endif // TP_ESP32_REMOTE_TEMPLATE_SENSORS_H
