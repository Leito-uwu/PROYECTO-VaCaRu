#ifndef SISTEMA_CLASIFICACION_H
#define SISTEMA_CLASIFICACION_H

#include <Arduino.h>
#include <AccelStepper.h>
#include <LiquidCrystal_I2C.h>

// Pines
#define IN1               13
#define IN2               12
#define IN3               14
#define IN4               23
#define MAXVELOCIDAD      500      
#define ACELERACION       200
#define BOTON_ROJO        2
#define BOTON_VERDE       4
#define PIN_SENSOR_IR     34
#define S0                32
#define S1                33
#define S2                25
#define S3                26
#define OUT_PIN           19
#define BAUDRATE          115200
#define TIME_1            100
#define TIME_2            50
#define TIME_3            300
#define TIME_4            500
#define TIME_5            1000
#define TIME_6            1500


// Constantes
const float diametroRuedaCm = 6.0;
const int stepsPerRevolution = 2048;
const int pasos_1= -1000000;
const int numMuestras = 10;
const float distanciaRojoCm = 23.0;
const float distanciaAzulCm = 22.0;
const float distanciaVerdeCm = 25.0;
const byte I2C_ADDRESS = 0x08;
const double pi = 3.1416;

// Estados
enum EstadoMotor {
  ESPERANDO_INICIO,
  GIRANDO_NORMAL,
  AVANZANDO,
  LEYENDO_COLOR,
  AVANZANDO_FINAL,
  ENVIANDO_COLOR,
  ESPERANDO_COMANDO
};

// Funciones principales
void inicializarSistema();
void ejecutarSistema();

#endif
