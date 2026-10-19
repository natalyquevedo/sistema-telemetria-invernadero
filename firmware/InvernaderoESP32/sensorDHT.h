#ifndef SENSOR_DHT_H
#define SENSOR_DHT_H

#include <Arduino.h>
#include <DHT.h>

class SensorDHT {
private:
    uint8_t _pin;
    uint8_t _type;
    DHT _dht;

public:
    SensorDHT(uint8_t pin, uint8_t type = DHT11);
    void begin();
    float readTemperature();
    float readHumidity();
    int verificarError(float valor);
};

#endif