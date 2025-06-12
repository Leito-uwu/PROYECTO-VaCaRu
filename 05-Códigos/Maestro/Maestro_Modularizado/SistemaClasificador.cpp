#include "SistemaClasificacion.h"

// LCD y Motor
LiquidCrystal_I2C lcd(0x27, 16, 2);
AccelStepper stepper(AccelStepper::FULL4WIRE, IN1, IN3, IN2, IN4);

// Variables globales
EstadoMotor estadoActual = ESPERANDO_INICIO;
int colorDetectado = -1;
bool sistemaIniciado = false;
bool emergenciaActiva = false;
int contadorBotellas = 0;

// Configuración inicial
void inicializarSistema() {
  Serial.begin(BAUDRATE);
  Wire.begin();
  lcd.begin(16, 2);
  lcd.backlight();

  pinMode(BOTON_ROJO, INPUT);
  pinMode(BOTON_VERDE, INPUT);
  pinMode(PIN_SENSOR_IR, INPUT);

  pinMode(S0, OUTPUT); pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT); pinMode(S3, OUTPUT);
  pinMode(OUT_PIN, INPUT);
  digitalWrite(S0, HIGH);
  digitalWrite(S1, LOW);

  lcd.clear();
  lcd.print("Presione Verde");
  lcd.setCursor(0, 1);
  lcd.print("para iniciar");

  stepper.setMaxSpeed(MAXVELOCIDAD);
  stepper.setAcceleration(ACELERACION);
}

// --- Auxiliares ---
bool hayObjeto() {
  return digitalRead(PIN_SENSOR_IR) == LOW;
}

unsigned long iirFilter(bool s2, bool s3, float alpha = 0.2) {
  digitalWrite(S2, s2);
  digitalWrite(S3, s3);
  delay(TIME_1);
  unsigned long y = 0;
  for (int i = 0; i < numMuestras; i++) {
    unsigned long lectura = pulseIn(OUT_PIN, LOW);
    y = (i == 0) ? lectura : alpha * lectura + (1 - alpha) * y;
    delay(TIME_2);
  }
  return y;
}

int detectarColor() {
  unsigned long red = iirFilter(LOW, LOW);
  unsigned long green = iirFilter(HIGH, HIGH);
  unsigned long blue = iirFilter(LOW, HIGH);

  Serial.print("ROJO: "); 
  Serial.println(red);
  Serial.print("VERDE: "); 
  Serial.println(green);
  Serial.print("AZUL: "); 
  Serial.println(blue);

  if (red < green && red < blue) return 0;
  else if (blue < red && blue < green) return 1;
  else if (green < red && green < blue) return 2;
  else return -1;
}

int calcularPasos(float distanciaCm) {
  float circunferencia = pi * diametroRuedaCm;
  return distanciaCm / (circunferencia / stepsPerRevolution);
}

void mostrarContador() {
  lcd.setCursor(0, 1);
  lcd.print("Botellas: ");
  lcd.print(contadorBotellas);
  lcd.print("   ");
}

// --- Ejecución del sistema ---
void ejecutarSistema() {
  if (estadoActual == ESPERANDO_INICIO) {
    if (digitalRead(BOTON_VERDE) == HIGH) {
      sistemaIniciado = true;
      emergenciaActiva = false;
      estadoActual = GIRANDO_NORMAL;
      lcd.clear(); lcd.print("Esperando objeto");
      mostrarContador();
      stepper.move(pasos_1);
      delay(TIME_3);
    }
    return;
  }

  if (digitalRead(BOTON_ROJO) == HIGH && !emergenciaActiva) {
    emergenciaActiva = true;
    lcd.clear(); 
    lcd.print("EMERGENCIA!");
    lcd.setCursor(0, 1); 
    lcd.print("Sistema detenido");
    stepper.stop();
    Wire.beginTransmission(I2C_ADDRESS);
    Wire.write(3);
    Wire.endTransmission();
    delay(TIME_4);
    return;
  }
  
  if (digitalRead(BOTON_ROJO) == HIGH && emergenciaActiva) {
    emergenciaActiva = false;
    lcd.clear(); 
    lcd.print("Reactivando...");
    delay(TIME_5);
    lcd.clear(); 
    lcd.print("Esperando objeto");
    mostrarContador();
    stepper.move(pasos_1);
    estadoActual = GIRANDO_NORMAL;
    Wire.beginTransmission(I2C_ADDRESS);
    Wire.write(4);
    Wire.endTransmission();
    delay(TIME_4);
  }

  if (!sistemaIniciado || emergenciaActiva) return;

  switch (estadoActual) {
    case GIRANDO_NORMAL:
      if (hayObjeto()) {
        lcd.clear(); 
        lcd.print("Objeto Detectado");
        stepper.stop();
        while (stepper.isRunning()) stepper.run();
        lcd.clear(); 
        lcd.print("Avance");
        mostrarContador();
        stepper.move(-calcularPasos(36));
        estadoActual = AVANZANDO;
      }
      break;

    case AVANZANDO:
      if (stepper.distanceToGo() == 0) {
        lcd.clear(); 
        lcd.print("Detectando Color");
        colorDetectado = detectarColor();
        lcd.setCursor(0, 1);
        if (colorDetectado == 0) 
          lcd.print("Color: ROJO");
          else if (colorDetectado == 1) 
            lcd.print("Color: AZUL");
          else if (colorDetectado == 2) 
            lcd.print("Color: VERDE");
          else lcd.print("Color: ???");

        delay(TIME_6);
        lcd.clear(); lcd.print("Avance Final");

        float distanciaFinal = (colorDetectado == 0) ? distanciaRojoCm :
                               (colorDetectado == 1) ? distanciaAzulCm :
                               (colorDetectado == 2) ? distanciaVerdeCm : distanciaRojoCm;

        stepper.move(-calcularPasos(distanciaFinal));
        estadoActual = AVANZANDO_FINAL;
      }
      break;

    case AVANZANDO_FINAL:
      if (stepper.distanceToGo() == 0) {
        lcd.clear(); lcd.print("Recogiendo...");
        contadorBotellas++;
        mostrarContador();
        delay(TIME_6);
        estadoActual = ENVIANDO_COLOR;
      }
      break;

    case ENVIANDO_COLOR:
      Wire.beginTransmission(I2C_ADDRESS);
      Wire.write(colorDetectado);
      Wire.endTransmission();
      estadoActual = ESPERANDO_COMANDO;
      break;

    case ESPERANDO_COMANDO:
      Wire.requestFrom(I2C_ADDRESS, 1);
      if (Wire.available()) {
        byte comando = Wire.read();
        if (comando == 1) {
          lcd.clear(); 
          lcd.print("Esperando objeto");
          mostrarContador();
          stepper.move(pasos_1);
          estadoActual = GIRANDO_NORMAL;
        }
      }
      break;
  }

  stepper.run();
}
