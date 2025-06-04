#include <ESP32Servo.h>
#include <EEPROM.h>

// Crear 5 objetos Servo
Servo servo1; 
Servo servo2;
Servo servo3;
Servo servo4;
Servo servo5;

// Definir los pines donde están conectados los servos
const int pinServo1 = 18;
const int pinServo2 = 19;
const int pinServo3 = 23;
const int pinServo4 = 25;
const int pinServo5 = 26;

// Variables de posiciones
int pos1, pos2, pos3, pos4, pos5;

// Direcciones en EEPROM para cada servo
const int addr1 = 0;
const int addr2 = 4;
const int addr3 = 8;
const int addr4 = 12;
const int addr5 = 16;

void setup() {
  Serial.begin(9600);

  servo1.attach(pinServo1);
  servo2.attach(pinServo2);
  servo3.attach(pinServo3);
  servo4.attach(pinServo4);
  servo5.attach(pinServo5);

  // Leer las últimas posiciones guardadas en EEPROM
  EEPROM.get(addr1, pos1);
  EEPROM.get(addr2, pos2);
  EEPROM.get(addr3, pos3);
  EEPROM.get(addr4, pos4);
  EEPROM.get(addr5, pos5);

  // Validar ángulos (por seguridad)
  pos1 = constrain(pos1, 0, 180);
  pos2 = constrain(pos2, 0, 180);
  pos3 = constrain(pos3, 0, 180);
  pos4 = constrain(pos4, 0, 180);
  pos5 = constrain(pos5, 0, 180);

  // Mover servos a las posiciones guardadas
  servo1.write(pos1);
  servo2.write(pos2);
  servo3.write(pos3);
  servo4.write(pos4);
  servo5.write(pos5);

  delay(500);

  Serial.println("Controla el brazo con las siguientes teclas:");
  Serial.println("Servo1: 'a' aumenta, 'z' disminuye");
  Serial.println("Servo2: 's' aumenta, 'x' disminuye");
  Serial.println("Servo3: 'd' aumenta, 'c' disminuye");
  Serial.println("Servo4: 'f' aumenta, 'v' disminuye");
  Serial.println("Servo5: 'g' aumenta, 'b' disminuye");
}

void moverServo(Servo& servo, int& pos, int angulo, int addr) {
  pos = constrain(angulo, 0, 180);    // Limita entre 0° y 180°
  servo.write(pos);                   // Mueve el servo
  EEPROM.put(addr, pos);              // Guarda nueva posición en EEPROM
  delay(20);                           // Pequeño delay para estabilidad
}

void loop() {
  if (Serial.available()) {
    char comando = Serial.read();  // Leer el carácter (sin esperar Enter)

    if (comando == '\n' || comando == '\r') return;  // Ignorar saltos de línea si llegan

    switch (comando) {
      case 'a': moverServo(servo1, pos1, pos1 + 5, addr1); break;
      case 'z': moverServo(servo1, pos1, pos1 - 5, addr1); break;
      case 's': moverServo(servo2, pos2, pos2 + 5, addr2); break;
      case 'x': moverServo(servo2, pos2, pos2 - 5, addr2); break;
      case 'd': moverServo(servo3, pos3, pos3 + 5, addr3); break;
      case 'c': moverServo(servo3, pos3, pos3 - 5, addr3); break;
      case 'f': moverServo(servo4, pos4, pos4 + 5, addr4); break;
      case 'v': moverServo(servo4, pos4, pos4 - 5, addr4); break;
      case 'g': moverServo(servo5, pos5, pos5 + 5, addr5); break;
      case 'b': moverServo(servo5, pos5, pos5 - 5, addr5); break;
    }
  }
}
