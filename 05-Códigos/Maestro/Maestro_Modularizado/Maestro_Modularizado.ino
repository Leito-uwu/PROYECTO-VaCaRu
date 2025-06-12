#include <Wire.h>
#include <AccelStepper.h>
#include <LiquidCrystal_I2C.h>
#include "SistemaClasificacion.h"

void setup() {
  inicializarSistema();
}

void loop() {
  ejecutarSistema();
}
