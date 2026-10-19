#include "telemetria.h"

Telemetria::Telemetria(const char* ssid, const char* pass, const char* broker, int port, const char* clientId)
    : _ssid(ssid), _pass(pass), _broker(broker), _port(port), _clientId(clientId), 
      _mqtt(_espClient), _lastReconnectMillis(0) {}

void Telemetria::begin() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(_ssid, _pass);
    _mqtt.setServer(_broker, _port);
    _mqtt.setBufferSize(256);
}

void Telemetria::mantenerConexion(unsigned long currentMillis) {
    if (WiFi.status() == WL_CONNECTED) {
        if (!_mqtt.connected() && (currentMillis - _lastReconnectMillis >= _reconnectInterval)) {
            _lastReconnectMillis = currentMillis;
            if (_mqtt.connect(_clientId)) {
                _mqtt.subscribe("invernadero/bomba/control");
            }
        }
        if (_mqtt.connected()) {
            _mqtt.loop();
        }
    }
}

void Telemetria::publicarValor(const char* topic, float valor, int precision) {
    if (_mqtt.connected()) {
        char buffer[16];
        dtostrf(valor, 1, precision, buffer);
        _mqtt.publish(topic, buffer);
    }
}

void Telemetria::publicarJSON(const char* topic, const char* nombreSensor, float valor, int error) {
    if (_mqtt.connected()) {
        char payload[128];
        snprintf(payload, sizeof(payload), "{\"sensor\":\"%s\",\"valor\":%.2f,\"error\":%d}", nombreSensor, valor, error);
        _mqtt.publish(topic, payload);
    }
}

void Telemetria::setCallback(MQTTCallback callback) {
    _mqtt.setCallback(callback);
}

void Telemetria::suscribir(const char* topic) {
    if (_mqtt.connected()) {
        _mqtt.subscribe(topic);
    }
}

bool Telemetria::estaConectado() {
    return WiFi.isConnected() && _mqtt.connected();
}
