# Sistema de Telemetría Ambiental y Control de Riego Bidireccional IoT para Invernadero
**Taller Práctico: Del Prototipo al Producto — Encerramiento Físico y Comunicación Bidireccional**

**Nataly Quevedo, Reichel Arias**  
Facultad de Ingeniería, Programa de Ingeniería de Sistemas y Computación  
Docente: Ing. Camilo Andrés Mendivelso Fajardo  
Noviembre de 2026  

---

**Resumen—** El presente proyecto documenta el desarrollo y validación de una estación de telemetría y control microclimático para invernadero basada en el microcontrolador ESP32, un broker MQTT (EMQX) y Node-RED. La solución trasciende la monitorización ambiental pasiva al integrar un canal de control bidireccional capaz de gobernar el accionamiento de una bomba de riego sumergible mediante un módulo relé desacoplado, adquirir y discriminar variables agroclimáticas (temperatura ambiente, humedad relativa del aire y humedad volumétrica del suelo) mediante programación no bloqueante en C++, y persistir la serie temporal en una base de datos relacional (MySQL). Asimismo, se despliega un panel de supervisión interactivo con indicadores tipo gauge, gráficos de tendencia temporal, sistema de alarmas por histéresis y conmutación de mando manual/automático. Finalmente, se establecen los criterios de diseño para el encerramiento físico hermético, garantizando protección de la electrónica frente a condensación por gradientes térmicos y mitigación del ruido inductivo generado por el motor de la bomba.

**Palabras clave—** Adquisición Ambiental, Bomba de Riego, Control Bidireccional, ESP32, Invernadero IoT, Módulo Relé, MQTT, MySQL, Node-RED.

---

## I. OBJETIVOS

Implementar un sistema de telemetría ambiental y control de riego bidireccional mediante el protocolo MQTT desacoplado en tópicos de adquisición multicapa, mando de potencia y confirmación de estado sobre un microcontrolador ESP32 integrado a Node-RED y base de datos relacional (MySQL), con el propósito de recolectar, supervisar y estabilizar variables críticas de microclima (temperatura, humedad del aire y humedad del suelo) en tiempo real, garantizando la fiabilidad operativa del microprocesador frente a transitorios de conmutación inductiva mediante aislamiento galvánico y un encerramiento físico resistente a la humedad.

---

## II. FUNCIONAMIENTO Y ARQUITECTURA DEL SISTEMA

El sistema opera bajo una arquitectura distribuida de cuatro capas coordinadas de manera asíncrona:

### A. Capa Física y Embebida (ESP32 y Transductores)
* **Adquisición Sensórica y Filtrado:** El ESP32 Dev Module muestrea periódicamente dos subsistemas transductores:
  * **Sensor DHT11 (GPIO 27):** Captura digital de temperatura ambiente y humedad relativa con biblioteca especializada y validación de redundancia cíclica.
  * **Sensor de Humedad de Suelo (GPIO 34 / ADC1_CH6):** Adquisición analógica sobre el conversor de 12 bits (rango 0 a 4095), asignado deliberadamente al bloque ADC1 para evitar colisiones con el transceptor Wi-Fi. Incorpora funciones de calibración y detección de fallos por circuito abierto (`verificarError`).
* **Telemetría No Bloqueante en C++:** Diseñado estrictamente bajo máquinas de estados temporizadas con `millis()`, sin uso de llamadas bloqueantes `delay()`. Cada 2000 ms se serializan los datos hacia el broker en dos modalidades:
  1. *Publicación escalar directa:* Tópicos independientes (`invernadero/temperatura`, `invernadero/humedad_aire`, `invernadero/humedad_suelo`) para alimentación inmediata de los medidores del Dashboard.
  2. *Serialización estructurada JSON:* Canal general `invernadero/telemetria` conteniendo clave de sensor, magnitud física y código de error de diagnóstico.
* **Canal de Control y Actuación Hidráulica:** Se implementó una clase modular independiente (`Bomba.h` / `Bomba.cpp`) que encapsula la activación de un módulo relé con optoacoplador (**GPIO 26**) y un LED indicador de conmutación (**GPIO 2**). El cliente MQTT suscribe el tópico `invernadero/bomba/control`; la función `callbackBomba` procesa comandos de texto (`ON`/`OFF`, `TRUE`/`FALSE`, `1`/`0`) en tiempo real, asegurando la conmutación segura del solenoide bajo lógica activa en bajo (`RELE_ACTIVO_BAJO = true`).
* **Aislamiento Eléctrico y Protección Inductiva:** Para mitigar el ruido electromagnético (EMI) y picos de fuerza contraelectromotriz (back-EMF) generados por el bobinado de la bomba de agua de 5V/12V, la etapa de potencia se encuentra aislada mediante el optoacoplador del módulo relé, alimentando el riel lógico de la ESP32 desde una fuente regulada independiente filtrada con capacitores cerámicos de 100 nF y electrolíticos de 470 $\mu$F.

### B. Capa de Transporte y Procesamiento (Node-RED y MySQL)
* **Broker MQTT:** Intermediario de mensajería sustentado sobre `broker.emqx.io` (puerto estándar 1883), garantizando entrega de paquetes asíncrona sin dependencia de direccionamiento IP local estático y habilitando conectividad entre redes heterogéneas.
* **Tubería Modular de Funciones:** El flujo en Node-RED procesa el flujo entrante mediante una secuencia de validación de frontera:
  * *Nodo Validador y Sanitizador:* Descarta paquetes corruptos o lecturas con código de error activo, verificando que los valores oscilen en rangos agronómicos plausibles (0 °C a 50 °C; 10% a 95% HR).
  * *Persistencia Parametrizada (`telemetria_iot`):* Ejecuta sentencias SQL preparadas (`INSERT INTO telemetria (sensor, valor) VALUES (?, ?)`) sobre la base de datos `iot`, protegiendo la persistencia contra inyecciones y garantizando una tasa de inserción continua y normalizada.
  * *Motor de Reglas y Alertas:* Evalúa en tiempo de ejecución umbrales de confort: disparo de notificaciones visuales (toast) ante temperaturas superiores a 28 °C o estrés hídrico en suelo.
* **Consola de Supervisión y Mando:** Despliegue de un panel web responsivo que agrupa indicadores tipo Gauge instantáneos, historial continuo en gráficas temporales lineales (temperatura y humedades) y un interruptor manual (`ui_switch`) con despacho inmediato de ráfagas MQTT hacia el actuador.

### C. Criterios de Diseño del Enclosure (Caja Protectora para Invernadero)
* **Protección contra Gradientes de Humedad y Rocío:** Debido a la alta saturación de vapor de agua en el ambiente interno del invernadero, la placa electrónica central (ESP32, placa perforada y reguladores) se alberga dentro de una caja con sellado perimetral estanco y empaque de silicona, evitando cortocircuitos por rocío matutino o condensación.
* **Aislamiento de la Etapa de Potencia:** El módulo relé y el cableado de la bomba se ubican en una sección físicamente segregada del microcontrolador sensible dentro de la caja, utilizando prensaestopas impermeables (glands) para la salida de los cables hacia el depósito hídrico y hacia el suelo.
* **Distribución de Transductores:** El sensor DHT11 se extiende al exterior del encerramiento a media altura en una zona con circulación de aire pero protegida de la radiación solar directa, mientras que la sonda de suelo penetra el sustrato radicular a 5 cm de profundidad mediante conductores apantallados.
* **Mantenibilidad y Diagnóstico Externo:** La carcasa incorpora indicadores lumínicos visibles desde el exterior (LED de estado de red y LED de bomba) y un puerto de servicio accesible para reprogramación sin comprometer el sellado del circuito.

---

## III. CONCLUSIONES

1. La segregación modular del firmware en clases orientadas a objetos (`Bomba`, `SensorDHT`, `SensorSuelo` y `Telemetria`) simplificó la integración del actuador relé, desacoplando la lógica de conmutación de hardware respecto a la capa de comunicaciones MQTT y asegurando un ciclo de ejecución no bloqueante basado enteramente en `millis()`.
2. El esquema de comunicación bidireccional sobre tópicos MQTT permitió un lazo cerrado confiable: mientras el microcontrolador publica de forma periódica variables físicas cada 2 segundos sin saturar la red, el canal de control suscrito (`invernadero/bomba/control`) intercepta de manera instantánea órdenes de riego manuales o automáticas emitidas desde el Dashboard de Node-RED.
3. El uso de sentencias SQL preparadas en Node-RED y el filtrado por código de error en la ESP32 aseguraron la integridad de la serie de tiempo en MySQL, evitando el almacenamiento de lecturas espurias ante eventuales desconexiones de sensores.
4. El encerramiento protector estructurado con aislamiento galvánico del relé y protección contra condensación garantiza la estabilidad del sistema embebido, eliminando bloqueos del procesador o caídas de tensión (brownouts) inducidas por la energización de la motobomba de riego.
