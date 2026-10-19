#include <Arduino.h>
#include "sensorDHT.h"
#include "SensorSuelo.h"
#include "telemetria.h"

// ==========================================
// CONFIGURACIÓN DE RED Y BROKER (EMQX)
// ==========================================
const char* WIFI_SSID   = "Naty";
const char* WIFI_PASS   = "natt123Q";
const char* MQTT_BROKER = "broker.emqx.io";
const int   MQTT_PORT   = 1883;

// ==========================================
// TÓPICOS MQTT (COINCIDEN CON NODE-RED)
// ==========================================
const char* TOPIC_TEMP   = "invernadero/temperatura";
const char* TOPIC_HUM    = "invernadero/humedad_aire";
const char* TOPIC_SUELO  = "invernadero/humedad_suelo";
const char* TOPIC_JSON   = "invernadero/telemetria";

// ==========================================
// PINES
// ==========================================
const int PIN_DHT   = 27;  // Sensor de aire DHT11
const int PIN_SUELO = 34;  // Sensor analógico de suelo (ADC1 GPIO 34)
const int PIN_BOMBA = 2;   // Actuador bomba de riego / LED indicador

// ==========================================
// INSTANCIAS DE CLASES
// ==========================================
SensorDHT   sensorAire(PIN_DHT, DHT11);
SensorSuelo sensorSuelo(PIN_SUELO);
Telemetria  telemetria(WIFI_SSID, WIFI_PASS, MQTT_BROKER, MQTT_PORT, "ESP32_Invernadero_UNAL");

// ==========================================
// TEMPORIZACIÓN NO BLOQUEANTE (millis)
// ==========================================
unsigned long previousMillis = 0;
const unsigned long interval = 2000; // Enviar cada 2 segundos

// Callback para recibir comandos de la bomba desde Node-RED
void callbackBomba(char* topic, uint8_t* payload, unsigned int length) {
    String msg = "";
    for (unsigned int i = 0; i < length; i++) {
        msg += (char)payload[i];
    }
    msg.trim();

    Serial.print("[COMANDO BOMBA RECIBIDO]: ");
    Serial.println(msg);

    if (msg == "ON") {
        digitalWrite(PIN_BOMBA, HIGH);
        Serial.println(">>> BOMBA: ACTIVADA");
    } else if (msg == "OFF") {
        digitalWrite(PIN_BOMBA, LOW);
        Serial.println(">>> BOMBA: DESACTIVADA");
    }
}

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("\n==========================================");
    Serial.println("   SISTEMA DE TELEMETRIA INVERNADERO      ");
    Serial.println("==========================================");

    pinMode(PIN_BOMBA, OUTPUT);
    digitalWrite(PIN_BOMBA, LOW);

    sensorAire.begin();
    sensorSuelo.begin();
    telemetria.setCallback(callbackBomba);
    telemetria.begin();

    Serial.println("Iniciando servicios... Conectando a WiFi Naty y MQTT EMQX.");
}

void loop() {
    unsigned long currentMillis = millis();
    telemetria.mantenerConexion(currentMillis);

    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;

        // 1. Lectura de Temperatura
        float temp = sensorAire.readTemperature();
        int errTemp = sensorAire.verificarError(temp);

        // 2. Lectura de Humedad del Aire
        float humAire = sensorAire.readHumidity();
        int errHum = sensorAire.verificarError(humAire);

        // 3. Lectura de Humedad del Suelo
        int rawSuelo = sensorSuelo.leer();
        int errSuelo = sensorSuelo.verificarError(rawSuelo);
        float humSuelo = (errSuelo == 0) ? sensorSuelo.leerHumedad(rawSuelo) : 0.0f;

        // Impresión en Monitor Serie
        Serial.print("Temp: ");
        Serial.print(temp);
        Serial.print(" °C | Hum Aire: ");
        Serial.print(humAire);
        Serial.print(" % | Hum Suelo: ");
        Serial.print(humSuelo);
        Serial.print(" % | MQTT: ");
        Serial.println(telemetria.estaConectado() ? "CONECTADO" : "CONECTANDO...");

        // PUBLICACIÓN A NODE-RED:
        // A) Publicar valores numéricos a los tópicos directos de los medidores
        if (errTemp == 0) {
            telemetria.publicarValor(TOPIC_TEMP, temp);
        }
        if (errHum == 0) {
            telemetria.publicarValor(TOPIC_HUM, humAire);
        }
        if (errSuelo == 0) {
            telemetria.publicarValor(TOPIC_SUELO, humSuelo);
        }

        // B) Publicar JSON estructurado al tópico general (Capa 1.4 del taller)
        telemetria.publicarJSON(TOPIC_JSON, "temperatura", temp, errTemp);
        telemetria.publicarJSON(TOPIC_JSON, "humedad_aire", humAire, errHum);
        telemetria.publicarJSON(TOPIC_JSON, "humedad_suelo", humSuelo, errSuelo);
    }
}