# Sistema de Telemetría Ambiental IoT de Extremo a Extremo

> **Taller Práctico Integrado:** Monitoreo y control microclimático para invernadero con ESP32, MQTT (EMQX), Node-RED y MySQL.

---

## 📋 Descripción del Proyecto

Este proyecto implementa una solución completa de Internet de las Cosas (IoT) dividida en cuatro capas arquitectónicas, diseñada para adquirir variables ambientales (temperatura, humedad del aire y humedad del suelo), procesarlas de forma asíncrona, persistirlas en una base de datos relacional normalizada y visualizarlas en tiempo real a través de un Dashboard interactivo.

---

## 🏛️ Arquitectura del Sistema

```
[ Sensor DHT11 ] -----\
                       +---> [ ESP32 (Edge) ] --(WiFi / MQTT)--> [ Broker EMQX ]
[ Sensor de Suelo ] --/          |                                     |
                                 |                                     v
[ Bomba / Relé ] <---------------+ <--(Comandos MQTT)--------- [ Node-RED ]
                                                                   |       |
                                      +----------------------------+       |
                                      v                                    v
                              [ MySQL / MariaDB ]               [ Node-RED Dashboard ]
                              (Tabla telemetria)               (Gauges, Charts, Alertas)
```

### 1. Capa 1: Adquisición de Datos (Hardware y Edge)
* **Microcontrolador:** ESP32 Dev Module.
* **Sensores:**
  * **DHT11 (GPIO 27):** Monitoreo de temperatura y humedad relativa del aire.
  * **Sensor de Suelo (GPIO 34 - ADC1):** Medición analógica calibrada de la humedad del sustrato.
* **Actuador:** Relé / LED indicador (GPIO 2) para el encendido y apagado de la bomba de riego.
* **Firmware No Bloqueante:** Programación orientada a objetos en C++ basada estrictamente en `millis()`, sin uso de `delay()`.
* **Manejo de Errores:** Métodos `verificarError()` en cada clase de sensor; si un sensor falla o se desconecta, se reporta un código de error específico (`101` para DHT11, `102` para suelo) en lugar de inyectar datos nulos o valores basura.

### 2. Capa 2: Procesamiento y Enrutamiento (Node-RED)
* Recepción de telemetría vía MQTT desde el broker `broker.emqx.io`.
* Nodos de función en JavaScript optimizados para la sanitización de lecturas antes de su almacenamiento.
* Evaluación de umbrales críticos de microclima:
  * Alerta de temperatura si supera los **28°C**.
  * Alerta de humedad si sale del rango operativo del **30% al 50%**.

### 3. Capa 3: Persistencia de Datos (MySQL / MariaDB)
* Base de datos relacional: `iot`.
* Tabla normalizada `telemetria`:
  * `id`: Entero autoincremental (Clave primaria).
  * `sensor`: Nombre/tipo de variable (`VARCHAR`).
  * `valor`: Lectura física en coma flotante (`FLOAT`).
  * `fecha`: Marca temporal generada automáticamente por el motor (`TIMESTAMP DEFAULT CURRENT_TIMESTAMP`).
* Inserción en tiempo real mediante **sentencias `INSERT` parametrizadas** (`?`) para máxima seguridad contra inyecciones SQL.

### 4. Capa 4: Visualización (Node-RED Dashboard)
* **Gauges:** Lecturas actuales e instantáneas de temperatura, humedad del aire y humedad del suelo.
* **Charts (Gráficas de líneas):** Histórico continuo de las últimas horas para análisis de perturbaciones térmicas y tendencias.
* **Panel de Notificaciones (UI Text & Toast):** Alertas en tiempo real cuando se sobrepasan los umbrales definidos o cuando un sensor reporta desconexión.
* **Control de Bomba (UI Switch):** Interruptor para encendido/apagado manual del sistema de riego vía MQTT.

---

## 📁 Estructura del Repositorio

```
sistema-telemetria-invernadero/
├── firmware/
│   └── InvernaderoESP32/
│       ├── InvernaderoESP32.ino   # Sketch principal
│       ├── telemetria.h           # Cabecera de gestión MQTT/WiFi
│       ├── telemetria.cpp         # Implementación de telemetria
│       ├── SensorSuelo.h          # Clase sensor analógico de suelo
│       ├── SensorSuelo.cpp        # Implementación calibrada y errores
│       ├── sensorDHT.h            # Clase sensor DHT11
│       └── sensorDHT.cpp          # Implementación sensor DHT11
├── node-red/
│   └── flows_invernadero.json     # Exportación completa del flujo
├── database/
│   └── init_db.sql                # Script de creación de la base de datos
├── README.md                      # Documentación del proyecto
└── .gitignore                     # Filtros para control de versiones
```

---

## 🚀 Guía de Puesta en Marcha

### 1. Base de Datos
Ejecutar el script [database/init_db.sql](database/init_db.sql) en MySQL Workbench, DBeaver o phpMyAdmin:
```sql
CREATE DATABASE IF NOT EXISTS iot;
USE iot;

CREATE TABLE IF NOT EXISTS telemetria (
    id INT AUTO_INCREMENT PRIMARY KEY,
    sensor VARCHAR(50) NOT NULL,
    valor FLOAT NOT NULL,
    fecha TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
```

### 2. Node-RED
1. Instalar la paleta de MySQL:
   * Menú **☰** $\rightarrow$ **Manage palette** $\rightarrow$ Instalar `node-red-node-mysql`.
2. Importar el flujo:
   * Menú **☰** $\rightarrow$ **Import** $\rightarrow$ Seleccionar o pegar el contenido de [node-red/flows_invernadero.json](node-red/flows_invernadero.json).
3. Configurar credenciales de MySQL:
   * Editar el nodo **Guardar en MySQL**, ingresar usuario (`root`) y contraseña local.
4. Presionar **Deploy**. El panel web estará disponible en:
   ```text
   http://localhost:1880/ui
   ```

### 3. Firmware ESP32
1. Abrir Arduino IDE y cargar el archivo [firmware/InvernaderoESP32/InvernaderoESP32.ino](firmware/InvernaderoESP32/InvernaderoESP32.ino).
2. Instalar las librerías necesarias desde el Gestor de Librerías:
   * `PubSubClient` (por Nick O'Leary)
   * `DHT sensor library` (por Adafruit)
   * `Adafruit Unified Sensor`
3. Ajustar el nombre de red y contraseña Wi-Fi:
   ```cpp
   const char* WIFI_SSID   = "Tu_Red_WiFi";
   const char* WIFI_PASS   = "Tu_Contraseña";
   ```
4. Seleccionar la placa **ESP32 Dev Module**, el puerto serial correspondiente y subir el programa.
