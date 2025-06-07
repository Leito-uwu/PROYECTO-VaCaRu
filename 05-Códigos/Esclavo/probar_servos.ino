#include <ESP32Servo.h>

// Definir el pin donde está conectado el servo
const int servoPin = 18; // Puedes cambiarlo por el pin que uses

Servo miServo; // Crear objeto servo
int angulo = 90; // Ángulo inicial

void setup() {
  Serial.begin(115200);
  miServo.attach(servoPin); // Inicializar el servo
  Serial.println("Escribe un angulo entre 0 y 180:");
}

void loop() {
  if (Serial.available() > 0) { // Si hay datos en el monitor serial
    angulo = Serial.parseInt(); // Leer el número
    if (angulo >= 0 && angulo <= 180) {
      miServo.write(angulo); // Mover el servo al ángulo recibido
      Serial.print("Moviendo al angulo: ");
      Serial.println(angulo);
    } else {
      Serial.println("Por favor escribe un valor entre 0 y 180");
    }
  }
}
