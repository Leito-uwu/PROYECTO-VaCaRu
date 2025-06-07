#include <ESP32Servo.h>
#include <Wire.h> // Librería para I2C

// Crear servos
Servo servo1; // Garra
Servo servo2; // Muñeca
Servo servo3; // Codo
Servo servo4; // Hombro
Servo servo5; // Base

// Pines
const int pinServo1 = 18; // Garra
const int pinServo2 = 19; // Muñeca
const int pinServo3 = 23; // Codo
const int pinServo4 = 25; // Hombro
const int pinServo5 = 26; // Base

// Variables de posición iniciales (reposo)
int pos1 = 60;   // Garra
int pos2 = 140;  // Muñeca
int pos3 = 180;  // Codo
int pos4 = 55;   // Hombro
int pos5 = 85;   // Base

// Variables de control
String comando = "";
volatile bool comandoNuevo = false;
volatile int comandoI2C = -1;
bool procesoTerminado = false;

// Dirección del esclavo I2C
#define SLAVE_ADDRESS 0x08

// Función para mover gradual un servo
void moverServoGradual(Servo& servo, int& posActual, int destino, int paso, int tiempo_ms) {
  destino = constrain(destino, 0, 180);  // Limitar entre 0° y 180°

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
  destinoA = constrain(destinoA, 0, 180);
  destinoB = constrain(destinoB, 0, 180);

  while (posA != destinoA || posB != destinoB) {
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

// Inicializar servos
void setupServos() {
  servo1.attach(pinServo1); // Garra
  servo2.attach(pinServo2); // Muñeca
  servo3.attach(pinServo3); // Codo
  servo4.attach(pinServo4); // Hombro
  servo5.attach(pinServo5); // Base

  reposo();
}

// Función reposo: llevar brazo a posición inicial
void reposo() {
  moverServosParalelo(servo4, pos4, 55, servo3, pos3, 180, 1, 10); // Hombro y codo
  moverServosParalelo(servo1, pos1, 60, servo2, pos2, 140, 1, 5);  // Garra y muñeca
  moverServoGradual(servo5, pos5, 85, 1, 10);                     // Base
}

// Simulación recoger botella
void recogerBotella() {
  moverServosParalelo(servo2, pos2, 120, servo1, pos1, 0, 1, 10);   // Bajar muñeca y abrir garra
  moverServosParalelo(servo4, pos4, 70, servo3, pos3, 135, 1, 10);  // Bajar hombro y codo
  moverServoGradual(servo5, pos5, 175, 1, 10);                      // Abrir garra
  moverServosParalelo(servo4, pos4, 40, servo3, pos3, 153, 1, 10);  // Subir hombro y codo
  moverServoGradual(servo1, pos1, 40, 1, 10);                       // Cerrar garra
  moverServoGradual(servo4, pos4, 60, 1, 10);                       // Ajustar hombro
}

// Movimiento a contenedor ROJO
void depositarEnRojo() {
  moverServoGradual(servo5, pos5, 120, 1, 10); // Girar base hacia rojo
  moverServosParalelo(servo4, pos4, 5, servo3, pos3, 105, 1, 10); // Bajar hombro y codo
  moverServoGradual(servo1, pos1, 0, 1, 20); // Soltar garra
}

// Movimiento a contenedor AZUL
void depositarEnAzul() {
  moverServoGradual(servo5, pos5, 93, 1, 10); 
  moverServosParalelo(servo4, pos4, 5, servo3, pos3, 105, 1, 10);
  moverServoGradual(servo1, pos1, 0, 1, 20);
}

// Movimiento a contenedor VERDE
void depositarEnVerde() {
  moverServoGradual(servo5, pos5, 70, 1, 10); 
  moverServosParalelo(servo4, pos4, 5, servo3, pos3, 105, 1, 10);
  moverServoGradual(servo1, pos1, 0, 1, 20);
}

// Función que recibe datos I2C
void recibirDato(int byteCount) {
  if (Wire.available()) {
    comandoI2C = Wire.read(); // Leer comando recibido
    comandoNuevo = true;
  }
}

// Función para enviar respuesta al maestro
void enviarRespuesta() {
  if (procesoTerminado) {
    Wire.write(1);  // Terminado
    procesoTerminado = false;  // Resetear estado
  } else {
    Wire.write(0);  // Aún trabajando
  }
}

void setup() {
  Serial.begin(9600);
  setupServos();
  Serial.println("Brazo en reposo. Esperando comandos...");

  Wire.begin(SLAVE_ADDRESS); // Iniciar I2C como esclavo
  Wire.onReceive(recibirDato);
  Wire.onRequest(enviarRespuesta);

  reposo(); // Posición inicial
}

void loop() {
  if (comandoNuevo) {
    Serial.print("Comando recibido via I2C: ");
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

    reposo();  // Siempre volver a reposo después
    procesoTerminado = true; // Marcar que terminó
    comandoNuevo = false;    // Resetear bandera
  }
}
