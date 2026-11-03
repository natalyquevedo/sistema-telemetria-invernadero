#include "Bomba.h"

Bomba::Bomba(int pinRele, int pinLed, bool activoBajo)
    : _pinRele(pinRele), _pinLed(pinLed), _activoBajo(activoBajo) {}

void Bomba::begin() {
    pinMode(_pinRele, OUTPUT);
    pinMode(_pinLed, OUTPUT);
    apagar(); // Asegurar que arranque apagado para evitar falsos disparos
}

void Bomba::encender() {
    cambiarEstado(true);
}

void Bomba::apagar() {
    cambiarEstado(false);
}

void Bomba::cambiarEstado(bool encender) {
    int estadoRele = encender ? (_activoBajo ? LOW : HIGH)
                              : (_activoBajo ? HIGH : LOW);
    digitalWrite(_pinRele, estadoRele);
    digitalWrite(_pinLed, encender ? HIGH : LOW);
}