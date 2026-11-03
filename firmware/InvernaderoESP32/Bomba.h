#ifndef BOMBA_H
#define BOMBA_H

#include <Arduino.h>

class Bomba {
private:
    int _pinRele;
    int _pinLed;
    bool _activoBajo;

public:
    Bomba(int pinRele, int pinLed = 2, bool activoBajo = true);
    void begin();
    void encender();
    void apagar();
    void cambiarEstado(bool encender);
};

#endif