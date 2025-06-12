#ifndef BRAZO_H
#define BRAZO_H

#include <ESP32Servo.h>

// Pines
#define PIN_SERVO1       18
#define PIN_SERVO2       19
#define PIN_SERVO3       23
#define PIN_SERVO4       25
#define PIN_SERVO5       26
#define SLAVE_ADDRESS   0x08

// Variables de control
extern int comandoI2C;
extern bool comandoNuevo;
extern bool emergencia;
extern bool procesoTerminado;

// Funciones
void inicializarBrazo();
void procesarComando();
void moverServoGradual(Servo &servo, int &posActual, int destino, int paso, int tiempo_ms);
void moverServosParalelo(Servo &servoA, int &posA, int destinoA, Servo &servoB, int &posB, int destinoB, int paso, int tiempo_ms);
void recogerBotella();
void depositarEnRojo();
void depositarEnAzul();
void depositarEnVerde();
void reposo();

#endif
