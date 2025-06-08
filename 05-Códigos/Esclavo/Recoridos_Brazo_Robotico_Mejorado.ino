#include <ESP32Servo.h>

// Crear servos
Servo servo1;
Servo servo2;
Servo servo3;
Servo servo4;
Servo servo5;

// Pines
const int pinServo1 = 18;
const int pinServo2 = 19;
const int pinServo3 = 23;
const int pinServo4 = 25;
const int pinServo5 = 26;

// Variables de posición iniciales (reposo)
int pos1 = 60;   // Base
int pos2 = 135;  // Hombro
int pos3 = 180;  // Codo
int pos4 = 55;   // Muñeca
int pos5 = 90;   // Garra

String comando = "";

// Función para mover gradual
void moverServoGradual(Servo& servo, int& posActual, int destino, int paso, int tiempo_ms) {
  destino = constrain(destino, 0, 180);  // limitar entre 0-180°

  if (posActual < destino) {
    for (int pos = posActual; pos <= destino; pos += paso) {
      servo.write(pos);
      delay(tiempo_ms);
    }
  } else {
    for (int pos = posActual; pos >= destino; pos -= paso) {
      servo.write(pos);
      delay(tiempo_ms);
    }
  }
  posActual = destino;  // Actualizar posición final
}

// Función para mover dos servos en paralelo
void moverServosParalelo(Servo& servoA, int& posA, int destinoA,
                         Servo& servoB, int& posB, int destinoB,
                         int paso, int tiempo_ms) {
  destinoA = constrain(destinoA, 0, 180);  // Limitar entre 0-180°
  destinoB = constrain(destinoB, 0, 180);

  // Mover hasta que ambos lleguen
  while (posA != destinoA || posB != destinoB) {
    // Mover servo A
    if (posA < destinoA) posA += paso;
    else if (posA > destinoA) posA -= paso;
    posA = constrain(posA, 0, 180);  // Asegura límites
    servoA.write(posA);

    // Mover servo B
    if (posB < destinoB) posB += paso;
    else if (posB > destinoB) posB -= paso;
    posB = constrain(posB, 0, 180);
    servoB.write(posB);

    delay(tiempo_ms);
  }
}


// Inicializar servos
void setupServos() {
  servo1.attach(pinServo1);
  servo2.attach(pinServo2);
  servo3.attach(pinServo3);
  servo4.attach(pinServo4);
  servo5.attach(pinServo5);

  reposo();
}

// Función reposo personalizada
void reposo() {
  moverServoGradual(servo1, pos1, 60, 1, 10);
  moverServosParalelo(servo4, pos4, 55, servo3, pos3, 180, 1, 10);
  moverServoGradual(servo2, pos2, 135, 1, 10);
  moverServoGradual(servo5, pos5, 90, 1, 10);
}

// Simulación recoger botella
void recogerBotella() {
  moverServoGradual(servo1, pos1, 0, 5, 20);  // abrir garra
  moverServoGradual(servo2, pos2, 100, 5, 20);
  moverServosParalelo(servo4, pos4, 70, servo3, pos3, 135, 1, 10);
  moverServoGradual(servo5, pos5, 180, 1, 15);
  moverServoGradual(servo2, pos2, 115, 1, 20);
  moverServosParalelo(servo4, pos4, 38, servo3, pos3, 150, 1, 10);
  moverServoGradual(servo1, pos1, 40, 1, 20);  // cerrar
  moverServoGradual(servo4, pos4, 60, 1, 20);
}

// Movimiento a contenedor ROJO
void depositarEnRojo() {
  moverServoGradual(servo5, pos5, 125, 1, 10);
  moverServosParalelo(servo4, pos4, 5, servo3, pos3, 100, 1, 10);
  moverServoGradual(servo2, pos2, 110, 1, 20);
  moverServoGradual(servo1, pos1, 0, 1, 20);
}

// Movimiento a contenedor AZUL
void depositarEnAzul() {
  moverServoGradual(servo5, pos5, 100, 1, 10);
  moverServosParalelo(servo4, pos4, 5, servo3, pos3, 100, 1, 10);
  moverServoGradual(servo2, pos2, 110, 1, 20);
  moverServoGradual(servo1, pos1, 0, 1, 20);
}

// Movimiento a contenedor VERDE
void depositarEnVerde() {
  moverServoGradual(servo5, pos5, 75, 1, 10);
  moverServosParalelo(servo4, pos4, 5, servo3, pos3, 100, 1, 10);
  moverServoGradual(servo2, pos2, 110, 1, 20);
  moverServoGradual(servo1, pos1, 0, 1, 20);
}

void setup() {
  Serial.begin(9600);
  setupServos();
  Serial.println("Brazo en reposo. Comandos:");
  Serial.println("R = ROJO");
  Serial.println("A = AZUL");
  Serial.println("V = VERDE");
  Serial.println("B = Recoger botella");
  Serial.println("I = Ir a Reposo");
}

void loop() {
  if (Serial.available()) {
    comando = Serial.readStringUntil('\n');
    comando.trim();  // Limpiar

    Serial.print("Comando recibido: ");
    Serial.println(comando);

    if (comando == "R") {
      recogerBotella();
      depositarEnRojo();
    } else if (comando == "A") {
      recogerBotella();
      depositarEnAzul();
    } else if (comando == "V") {
      recogerBotella();
      depositarEnVerde();
    } else if (comando == "B") {
      recogerBotella();
    } else if (comando == "I") {
      reposo();
    } else {
      Serial.println("Comando inválido. Usa R, A, V, B o I.");
    }
    delay(1000);
    reposo();  // Siempre volver al reposo después de la tarea
    reposo();  // Siempre volver al reposo después de la tarea
  }
}
