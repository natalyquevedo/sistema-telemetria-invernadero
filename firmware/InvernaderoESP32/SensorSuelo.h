#ifndef SENSOR_SUELO_H
#define SENSOR_SUELO_H

#include <Arduino.h>

class SensorSuelo {
private:
    int _pin;
    int _airValue;
    int _waterValue;

public:
    SensorSuelo(int pin, int airValue = 3500, int waterValue = 1400);
    void begin();
    int leer();
    float leerHumedad(int valorRaw);
    int verificarError(int valorRaw);
};

#endif
