#ifndef TELEMETRIA_H
#define TELEMETRIA_H

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

typedef std::function<void(char*, uint8_t*, unsigned int)> MQTTCallback;

class Telemetria {
private:
    const char* _ssid;
    const char* _pass;
    const char* _broker;
    int _port;
    const char* _clientId;
    WiFiClient _espClient;
    PubSubClient _mqtt;
    unsigned long _lastReconnectMillis;
    const unsigned long _reconnectInterval = 3000;

public:
    Telemetria(const char* ssid, const char* pass, const char* broker, int port, const char* clientId = "ESP32_Invernadero");
    void begin();
    void mantenerConexion(unsigned long currentMillis);
    
    // Publica valor numérico directamente al tópico del sensor (compatible con medidores directos)
    void publicarValor(const char* topic, float valor, int precision = 2);

    // Publica formato JSON estructurado según Capa 1.4 del taller
    void publicarJSON(const char* topic, const char* nombreSensor, float valor, int error);

    void setCallback(MQTTCallback callback);
    void suscribir(const char* topic);
    bool estaConectado();
};

#endif
