#include <Wire.h>
#include "brazo.h"

Servo servo1, servo2, servo3, servo4, servo5;
int pos1 = 60, pos2 = 140, pos3 = 180, pos4 = 55, pos5 = 85;

int comandoI2C = -1;
bool comandoNuevo = false;
bool emergencia = false;
bool procesoTerminado = false;

void recibirDato(int byteCount) {
  if (Wire.available()) {
    comandoI2C = Wire.read();
    comandoNuevo = true;

    if (comandoI2C == 3) {
      emergencia = true;
      Serial.println("Emergencia activada: Sistema detenido.");
    } else if (comandoI2C == 4) {
      emergencia = false;
      Serial.println("Emergencia desactivada: Volviendo a reposo.");
      reposo();
    }
  }
}

void enviarRespuesta() {
  if (procesoTerminado) {
    Wire.write(1);
    procesoTerminado = false;
  } else {
    Wire.write(0);
  }
}

void inicializarBrazo() {
  servo1.attach(PIN_SERVO1);
  servo2.attach(PIN_SERVO2);
  servo3.attach(PIN_SERVO3);
  servo4.attach(PIN_SERVO4);
  servo5.attach(PIN_SERVO5);

  Wire.begin(SLAVE_ADDRESS);
  Wire.onReceive(recibirDato);
  Wire.onRequest(enviarRespuesta);

  reposo();
}

void procesarComando() {
  Serial.print("Comando recibido: ");
  Serial.println(comandoI2C);

  if (comandoI2C == 0) {
    recogerBotella();
    depositarEnRojo();
  } else if (comandoI2C == 1) {
    recogerBotella();
    depositarEnAzul();
  } else if (comandoI2C == 2) {
    recogerBotella();
    depositarEnVerde();
  } else {
    Serial.println("Comando inválido.");
  }

  reposo();
  procesoTerminado = true;
  comandoNuevo = false;
}

void moverServoGradual(Servo &servo, int &posActual, int destino, int paso, int tiempo_ms) {
  destino = constrain(destino, 0, 180);
  if (posActual < destino) {
    for (int pos = posActual; pos <= destino; pos += paso) {
      if (emergencia) return;
      servo.write(pos);
      delay(tiempo_ms);
    }
  } else {
    for (int pos = posActual; pos >= destino; pos -= paso) {
      if (emergencia) return;
      servo.write(pos);
      delay(tiempo_ms);
    }
  }
  posActual = destino;
}

void moverServosParalelo(Servo &servoA, int &posA, int destinoA, Servo &servoB, int &posB, int destinoB, int paso, int tiempo_ms) {
  destinoA = constrain(destinoA, 0, 180);
  destinoB = constrain(destinoB, 0, 180);

  while ((posA != destinoA || posB != destinoB) && !emergencia) {
    if (posA < destinoA) posA += paso;
    else if (posA > destinoA) posA -= paso;
    posA = constrain(posA, 0, 180);
    servoA.write(posA);

    if (posB < destinoB) posB += paso;
    else if (posB > destinoB) posB -= paso;
    posB = constrain(posB, 0, 180);
    servoB.write(posB);

    delay(tiempo_ms);
  }
}

void recogerBotella() {
  moverServosParalelo(servo2, pos2, 120, servo1, pos1, 0, 1, 10);
  moverServosParalelo(servo4, pos4, 70, servo3, pos3, 135, 1, 10);
  moverServoGradual(servo5, pos5, 172, 1, 10);
  moverServosParalelo(servo4, pos4, 40, servo3, pos3, 153, 1, 10);
  moverServoGradual(servo1, pos1, 40, 1, 10);
  moverServosParalelo(servo4, pos4, 70, servo3, pos3, 150, 1, 10);
}

void depositarEnRojo() {
  moverServoGradual(servo5, pos5, 70, 1, 10);
  moverServosParalelo(servo4, pos4, 5, servo3, pos3, 105, 1, 10);
  moverServoGradual(servo1, pos1, 0, 1, 20);
}

void depositarEnAzul() {
  moverServoGradual(servo5, pos5, 93, 1, 10);
  moverServosParalelo(servo4, pos4, 5, servo3, pos3, 105, 1, 10);
  moverServoGradual(servo1, pos1, 0, 1, 20);
}

void depositarEnVerde() {
  moverServoGradual(servo5, pos5, 120, 1, 10);
  moverServosParalelo(servo4, pos4, 5, servo3, pos3, 105, 1, 10);
  moverServoGradual(servo1, pos1, 0, 1, 20);
}

void reposo() {
  moverServosParalelo(servo4, pos4, 55, servo3, pos3, 180, 1, 10);
  moverServosParalelo(servo1, pos1, 60, servo2, pos2, 140, 1, 15);
  moverServoGradual(servo5, pos5, 85, 1, 10);
}
