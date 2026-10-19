#include "SensorSuelo.h"

SensorSuelo::SensorSuelo(int pin, int airValue, int waterValue)
    : _pin(pin), _airValue(airValue), _waterValue(waterValue) {}

void SensorSuelo::begin() {
    pinMode(_pin, INPUT);
}

int SensorSuelo::leer() {
    return analogRead(_pin);
}

int SensorSuelo::verificarError(int valorRaw) {
    // Si el valor analógico es menor a 50 (corto) o mayor a 4090 (desconectado)
    if (valorRaw <= 50 || valorRaw >= 4090) {
        return 102; // Código de error: sensor de tierra desconectado
    }
    return 0; // Sin error
}

float SensorSuelo::leerHumedad(int valorRaw) {
    if (verificarError(valorRaw) != 0) {
        return -1.0f;
    }
    float hum = map(valorRaw, _airValue, _waterValue, 0, 100);
    if (hum < 0.0f) hum = 0.0f;
    if (hum > 100.0f) hum = 100.0f;
    return hum;
}
