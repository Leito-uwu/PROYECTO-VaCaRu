#include <Wire.h>
#include <ESP32Servo.h>
#include "brazo.h"

void setup() {
  Serial.begin(9600);
  inicializarBrazo();
  Serial.println("Brazo en reposo. Esperando comandos...");
}

void loop() {
  if (!emergencia) {
    if (comandoNuevo) {
      procesarComando();
    }
  }
}
