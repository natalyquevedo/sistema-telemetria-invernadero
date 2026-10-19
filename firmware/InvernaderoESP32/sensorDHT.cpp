#include "sensorDHT.h"

SensorDHT::SensorDHT(uint8_t pin, uint8_t type) : _pin(pin), _type(type), _dht(pin, type) {
}

void SensorDHT::begin() {
    _dht.begin();
}

float SensorDHT::readTemperature() {
    return _dht.readTemperature();
}

float SensorDHT::readHumidity() {
    return _dht.readHumidity();
}

int SensorDHT::verificarError(float valor) {
    if (isnan(valor)) {
        return 101; // Código de error: falla al leer sensor DHT11
    }
    return 0; // Sin error
}