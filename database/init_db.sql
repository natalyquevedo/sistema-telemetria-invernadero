-- ====================================================================
-- TALLER PRÁCTICO INTEGRADO: SISTEMA DE TELEMETRÍA AMBIENTAL IoT
-- Capa 3: Persistencia de Datos en Base de Datos Relacional (MySQL/MariaDB)
-- ====================================================================

CREATE DATABASE IF NOT EXISTS iot;
USE iot;

-- Tabla normalizada de telemetría
CREATE TABLE IF NOT EXISTS telemetria (
    id INT AUTO_INCREMENT PRIMARY KEY,
    sensor VARCHAR(50) NOT NULL,
    valor FLOAT NOT NULL,
    fecha TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Índices para optimización de consultas históricas
CREATE INDEX idx_sensor_fecha ON telemetria (sensor, fecha);
