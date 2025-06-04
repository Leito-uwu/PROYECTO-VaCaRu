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
int pos1 = 70;  // Base
int pos2 = 135; // Hombro
int pos3 = 180; // Codo
int pos4 = 55;  // Muñeca
int pos5 = 15;  // Garra

String comando = "";

// Función para mover gradual
void moverServoGradual(Servo& servo, int& posActual, int destino, int paso, int tiempo_ms) {
  destino = constrain(destino, 0, 180); // limitar entre 0-180°

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
  posActual = destino; // Actualizar posición final
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
  moverServoGradual(servo1, pos1, 70, 3, 20);  
  moverServoGradual(servo2, pos2, 135, 3, 20);
  moverServoGradual(servo3, pos3, 180, 3, 20);
  moverServoGradual(servo4, pos4, 55, 3, 20);
  moverServoGradual(servo5, pos5, 15, 3, 20);
}

// Simulación recoger botella
void recogerBotella() {
  moverServoGradual(servo1, pos1, 0, 3, 20);    // abrir garra
  moverServoGradual(servo2, pos2, 100, 3, 20);
  moverServoGradual(servo3, pos3, 140, 3, 20);
  moverServoGradual(servo4, pos4, 90, 3, 20);
  moverServoGradual(servo5, pos5, 100, 3, 20);  
  delay(500);
  moverServoGradual(servo2, pos2, 115, 2, 50);
  moverServoGradual(servo3, pos3, 150, 1, 50);
  moverServoGradual(servo4, pos4, 40, 1, 50);  
  moverServoGradual(servo1, pos1, 45, 3, 20);  // cerrar garra
  delay(500);
}

// Movimiento a contenedor ROJO
void depositarEnRojo() {
  moverServoGradual(servo2, pos2, 110, 3, 20);
  moverServoGradual(servo3, pos3, 100, 3, 20);
  moverServoGradual(servo4, pos4, 5, 3, 20);
  moverServoGradual(servo5, pos5, , 3, 20);
  moverServoGradual(servo1, pos1, 0, 3, 20);
  delay(500);
}

// Movimiento a contenedor AZUL
void depositarEnAzul() {
  moverServoGradual(servo2, pos2, 110, 3, 20);
  moverServoGradual(servo3, pos3, 100, 3, 20);
  moverServoGradual(servo4, pos4, 5, 3, 20);
  moverServoGradual(servo5, pos5, 60, 3, 20);
  moverServoGradual(servo1, pos1, 0, 3, 20);
  delay(500);
}

// Movimiento a contenedor VERDE
void depositarEnVerde() {
  moverServoGradual(servo1, pos1, 90, 3, 20);
  moverServoGradual(servo2, pos2, 70, 3, 20);
  moverServoGradual(servo3, pos3, 110, 3, 20);
  moverServoGradual(servo5, pos5, 35, 3, 20);  
  delay(500);
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
    comando.trim(); // Limpiar

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
    reposo(); // Siempre volver al reposo después de la tarea
  }
}
