#include <AccelStepper.h>

// definicion de pines motor
#define IN1 13
#define IN2 12
#define IN3 14
#define IN4 23

// motor paso a paso
AccelStepper stepper(AccelStepper::FULL4WIRE, IN1, IN3, IN2, IN4);

const int stepsPerRevolution = 2048;
const float diametroRuedaCm = 6.0;

// pines sensor infrarrojo FC-51
#define PIN_SENSOR_IR 34

// pines sensor TCS3200
#define S0 32
#define S1 33
#define S2 25
#define S3 26
#define OUT_PIN 19

// Umbrales
const float UMBRAL_BAJO = 4000;
const float UMBRAL_ALTO = 15000;

// Filtro IIR
const float alpha = 0.2;
float redFilt = 0, greenFilt = 0, blueFilt = 0;
String colorValidado = "";

// estados de control
enum EstadoMotor {
  GIRANDO_NORMAL,
  AVANZANDO_40CM,
  LEYENDO_COLOR,
  AVANZANDO_15CM,
  BLOQUEADO
};

EstadoMotor estadoActual = GIRANDO_NORMAL;

// --- FUNCIONES SENSOR COLOR ---

void configurarTCS() {
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(OUT_PIN, INPUT);
  digitalWrite(S0, HIGH);  // escala 20%
  digitalWrite(S1, LOW);
}

unsigned long leerColorRaw(bool s2, bool s3) {
  digitalWrite(S2, s2);
  digitalWrite(S3, s3);
  delay(10);  // espera para estabilizar
  unsigned long duracion = pulseIn(OUT_PIN, LOW);
  if (duracion == 0) return 0;
  return 1000000UL / duracion;
}

float aplicarFiltro(float entrada, float salidaAnterior) {
  return alpha * entrada + (1 - alpha) * salidaAnterior;
}

String detectarColor(float r, float g, float b) {
  if (r < UMBRAL_BAJO && g < UMBRAL_BAJO && b < UMBRAL_BAJO) return "ninguno";
  if (r > UMBRAL_ALTO || g > UMBRAL_ALTO || b > UMBRAL_ALTO) return "ruido";
  if (r > g && r > b) return "rojo";
  if (g > r && g > b) return "verde";
  if (b > r && b > g) return "azul";
  return "desconocido";
}

void realizarLecturaColor() {
  int rojo = 0, verde = 0, azul = 0;
  const int totalLecturas = 20;
  const int intervalo = 100;

  for (int i = 0; i < totalLecturas; i++) {
    redFilt   = aplicarFiltro(leerColorRaw(LOW, LOW), redFilt);
    greenFilt = aplicarFiltro(leerColorRaw(HIGH, HIGH), greenFilt);
    blueFilt  = aplicarFiltro(leerColorRaw(LOW, HIGH), blueFilt);

    String color = detectarColor(redFilt, greenFilt, blueFilt);
    Serial.println(color);

    if (color == "rojo") rojo++;
    else if (color == "verde") verde++;
    else if (color == "azul") azul++;

    delay(intervalo);
  }

  if (rojo >= 16) {
    Serial.println("Color Detectado: ROJO");
    colorValidado = "ROJO";
  } else if (verde >= 16) {
    Serial.println("Color Detectado: VERDE");
    colorValidado = "VERDE";
  } else if (azul >= 16) {
    Serial.println("Color Detectado: AZUL");
    colorValidado = "AZUL";
  } else {
    Serial.println("Color no detectado con certeza");
    colorValidado = "NINGUNO";
  }
}

// --- FUNCIONES SENSOR INFRARROJO ---

void configurarSensorIR() {
  pinMode(PIN_SENSOR_IR, INPUT);
}

bool hayObjeto() {
  return digitalRead(PIN_SENSOR_IR) == LOW;
}

// --- FUNCIONES MOTOR ---

int calcularPasos(float distanciaCm) {
  float circunferencia = 3.1416 * diametroRuedaCm;
  float distanciaPorPaso = circunferencia / stepsPerRevolution;
  int pasos = distanciaCm / distanciaPorPaso;
  return pasos;
}

void setup() {
  Serial.begin(115200);
  configurarSensorIR();
  configurarTCS();

  stepper.setMaxSpeed(500);
  stepper.setAcceleration(200);
  stepper.move(-1000000);  // avanzar indefinidamente
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
        int pasos40cm = -calcularPasos(35); 
        stepper.move(pasos40cm);
        estadoActual = AVANZANDO_40CM;
      }
      break;

    case AVANZANDO_40CM:
      if (stepper.distanceToGo() == 0) {
        Serial.println("40 cm completados, leyendo color...");
        realizarLecturaColor();

        Serial.println("Avanzando 15 cm mas...");
        int pasos15cm = -calcularPasos(25);
        stepper.move(pasos15cm);
        estadoActual = AVANZANDO_15CM;
      }
      break;

    case AVANZANDO_15CM:
      if (stepper.distanceToGo() == 0) {
        Serial.println("55 cm totales completados. Motor detenido. Esperando comando 'a' para reactivar...");
        estadoActual = BLOQUEADO;
      }
      break;

    case BLOQUEADO:
      if (Serial.available() > 0) {
        char comando = Serial.read();
        if (comando == 'a' || comando == 'A') {
          Serial.println("Reactivando sistema...");
          stepper.move(-1000000);
          estadoActual = GIRANDO_NORMAL;
        } else {
          Serial.println("Comando no valido. Envia 'a' para reactivar.");
        }
      }
      break;
  }

  stepper.run();
}
