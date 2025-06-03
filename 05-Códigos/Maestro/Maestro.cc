#include <Stepper.h>

// configuracion stepper
const int stepsPerRevolution = 2048;
const float diametroRuedaCm = 5.0;

Stepper myStepper(stepsPerRevolution, 13, 12, 14, 27);

// pines tcs3200
#define S0 32
#define S1 33
#define S2 25
#define S3 26
#define sensorOut 27

// pines fc-51
#define PIN_SENSOR_IR 34

// variables para frecuencia de color
int redFrequency = 0;
int greenFrequency = 0;
int blueFrequency = 0;

// configurar tcs3200
void configurarTCS() {
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(sensorOut, INPUT);

  digitalWrite(S0, HIGH);
  digitalWrite(S1, LOW);
}

// leer frecuencia de un color
int leerColor(bool s2, bool s3) {
  digitalWrite(S2, s2);
  digitalWrite(S3, s3);
  delay(100);
  return pulseIn(sensorOut, LOW);
}

// leer rojo verde azul
void leerColores() {
  redFrequency = leerColor(LOW, LOW);
  greenFrequency = leerColor(HIGH, HIGH);
  blueFrequency = leerColor(LOW, HIGH);
}

// detectar color predominante
String detectarColor() {
  if (redFrequency < greenFrequency && redFrequency < blueFrequency) {
    return "rojo";
  } else if (greenFrequency < redFrequency && greenFrequency < blueFrequency) {
    return "verde";
  } else if (blueFrequency < redFrequency && blueFrequency < greenFrequency) {
    return "azul";
  } else {
    return "no definido";
  }
}

// configurar fc-51
void configurarSensorIR() {
  pinMode(PIN_SENSOR_IR, INPUT);
}

// hay objeto?
bool hayObjeto() {
  return digitalRead(PIN_SENSOR_IR) == LOW;
}

// calcular pasos necesarios para distancia
int calcularPasos(float distanciaCm) {
  float circunferencia = 3.1416 * diametroRuedaCm;
  float distanciaPorPaso = circunferencia / stepsPerRevolution;
  int pasos = distanciaCm / distanciaPorPaso;
  return pasos;
}

// mover motor
void moverDistancia(float distanciaCm) {
  int pasosNecesarios = calcularPasos(distanciaCm);
  Serial.print("moviendose ");
  Serial.print(distanciaCm);
  Serial.println(" cm");
  myStepper.step(pasosNecesarios);
}

void setup() {
  Serial.begin(115200);
  myStepper.setSpeed(20);

  configurarTCS();
  configurarSensorIR();
}

void loop() {
  if (!hayObjeto()) {
    // motor avanza continuamente
    myStepper.step(5);  // mueve 5 pasos pequeños continuamente
  } else {
    // objeto detectado
    Serial.println("objeto detectado");

    leerColores();
    String colorDetectado = detectarColor();
    Serial.print("color detectado: ");
    Serial.println(colorDetectado);

    // avanza 9 cm despues de detectar y leer color
    moverDistancia(9.0);
    delay(2000);  // espera antes de volver a moverse
  }
}
// Fin de Programa
