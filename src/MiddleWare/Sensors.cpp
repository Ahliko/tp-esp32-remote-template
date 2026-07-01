//
// Created by Ahliko on 01/07/2026.
//

#include "Sensors.h"

Sensors::Sensors()  {
    m_wire = &Wire;
    m_ina237 = new INA237(m_wire);
    m_tmp126 = new TMP126();

}

void Sensors::init() {

}
