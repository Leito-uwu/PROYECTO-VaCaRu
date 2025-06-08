#include <Wire.h>
#include <AccelStepper.h>

// Definición de pines motor
#define IN1 13
#define IN2 12
#define IN3 14
#define IN4 23

// Motor paso a paso
AccelStepper stepper(AccelStepper::FULL4WIRE, IN1, IN3, IN2, IN4);

const int stepsPerRevolution = 2048;
const float diametroRuedaCm = 6.0;

// Pines sensor infrarrojo FC-51
#define PIN_SENSOR_IR 34

// Pines sensor TCS3200
#define S0 32
#define S1 33
#define S2 25
#define S3 26
#define OUT_PIN 19

// Variables para frecuencia
unsigned long redFreq, greenFreq, blueFreq;
int colorDetectado = -1;  // Guardamos el color detectado

// Número de muestras para promediar
const int numMuestras = 10;

// Umbral global de trabajo (rango válido)
const unsigned long UMBRAL_MIN = 180;   // frecuencia mínima válida
const unsigned long UMBRAL_MAX = 800;   // frecuencia máxima válida

// Dirección I2C del esclavo
const byte I2C_ADDRESS = 0x08;

// --- CONFIGURACIÓN SENSOR COLOR ---
void configurarTCS() {
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(OUT_PIN, INPUT);

  digitalWrite(S0, HIGH); // Frecuencia de salida 20%
  digitalWrite(S1, LOW);

  Serial.println("Sensor TCS3200 Configurado...");
}

// Función para leer frecuencia de un color específico
unsigned long leerFrecuencia(bool s2, bool s3) {
  digitalWrite(S2, s2);
  digitalWrite(S3, s3);
  delay(100);
  return pulseIn(OUT_PIN, LOW);
}

// Función para obtener el promedio de varias lecturas
unsigned long promedioFrecuencia(bool s2, bool s3) {
  unsigned long suma = 0;
  for (int i = 0; i < numMuestras; i++) {
    suma += leerFrecuencia(s2, s3);
    delay(50);
  }
  return suma / numMuestras;
}

// Función para detectar color y asignar valor numérico
int detectarColor() {
  redFreq = promedioFrecuencia(LOW, LOW);      // Rojo
  greenFreq = promedioFrecuencia(HIGH, HIGH);  // Verde
  blueFreq = promedioFrecuencia(LOW, HIGH);    // Azul

  Serial.print("Rojo: "); Serial.println(redFreq);
  Serial.print("Verde: "); Serial.println(greenFreq);
  Serial.print("Azul: "); Serial.println(blueFreq);

  // Verificar si los valores están dentro del umbral válido
  if ((redFreq < UMBRAL_MIN || redFreq > UMBRAL_MAX) ||
      (greenFreq < UMBRAL_MIN || greenFreq > UMBRAL_MAX) ||
      (blueFreq < UMBRAL_MIN || blueFreq > UMBRAL_MAX)) {
    return -1; // Color sin importancia
  }

  if (redFreq < greenFreq && redFreq < blueFreq) {
    return 0; // ROJO
  } else if (blueFreq < redFreq && blueFreq < greenFreq) {
    return 1; // AZUL
  } else if (greenFreq < redFreq && greenFreq < blueFreq) {
    return 2; // VERDE
  } else {
    return -1; // DESCONOCIDO
  }
}

// --- CONFIGURACIÓN SENSOR IR ---
void configurarSensorIR() {
  pinMode(PIN_SENSOR_IR, INPUT);
}

bool hayObjeto() {
  return digitalRead(PIN_SENSOR_IR) == LOW;
}

// --- MOTOR ---
int calcularPasos(float distanciaCm) {
  float circunferencia = 3.1416 * diametroRuedaCm;
  float distanciaPorPaso = circunferencia / stepsPerRevolution;
  int pasos = distanciaCm / distanciaPorPaso;
  return pasos;
}

// --- CONTROL ESTADOS MOTOR ---
enum EstadoMotor {
  GIRANDO_NORMAL,
  AVANZANDO_40CM,
  LEYENDO_COLOR,
  AVANZANDO_15CM,
  ENVIANDO_COLOR,
  ESPERANDO_COMANDO
};

EstadoMotor estadoActual = GIRANDO_NORMAL;

void setup() {
  Serial.begin(115200);
  Wire.begin(); // Iniciar I2C como maestro
  configurarSensorIR();
  configurarTCS();

  stepper.setMaxSpeed(500);
  stepper.setAcceleration(200);
  stepper.move(-1000000);  // Avanzar indefinidamente
  Serial.println("Sistema listo. Motor girando...");
}

void loop() {
  switch (estadoActual) {
    case GIRANDO_NORMAL:
      if (hayObjeto()) {
        Serial.println("Objeto detectado, deteniendo motor...");
        stepper.stop();
        while (stepper.isRunning()) {
          stepper.run();
        }

        Serial.println("Avanzando 40 cm...");
        int pasos40cm = -calcularPasos(36); 
        stepper.move(pasos40cm);
        estadoActual = AVANZANDO_40CM;
      }
      break;

    case AVANZANDO_40CM:
      if (stepper.distanceToGo() == 0) {
        Serial.println("40 cm completados, midiendo color...");
        colorDetectado = detectarColor();
        Serial.print("Color detectado (0=ROJO, 1=AZUL, 2=VERDE): ");
        Serial.println(colorDetectado);

        Serial.println("Avanzando 15 cm más...");
        int pasos15cm = -calcularPasos(21);
        stepper.move(pasos15cm);
        estadoActual = AVANZANDO_15CM;
      }
      break;

    case AVANZANDO_15CM:
      if (stepper.distanceToGo() == 0) {
        Serial.println("55 cm totales completados. Motor detenido.");
        estadoActual = ENVIANDO_COLOR;
      }
      break;

    case ENVIANDO_COLOR:
      // Enviar color por I2C al esclavo
      Wire.beginTransmission(I2C_ADDRESS);
      Wire.write(colorDetectado);
      Wire.endTransmission();
      Serial.println("Color enviado por I2C...");
      estadoActual = ESPERANDO_COMANDO;
      break;

    case ESPERANDO_COMANDO:
      Wire.requestFrom(I2C_ADDRESS, 1);
      if (Wire.available()) {
        byte comando = Wire.read();
        if (comando == 1) {
          Serial.println("Comando recibido: 1 -> Reactivando sistema...");
          stepper.move(-1000000);
          estadoActual = GIRANDO_NORMAL;
        }
      }
      break;
  }
  stepper.run();
}
