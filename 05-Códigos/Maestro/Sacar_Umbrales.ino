a// Pines sensor TCS3200
#define S0 32
#define S1 33
#define S2 25
#define S3 26
#define OUT_PIN 19

// Número de muestras por color
const int numMuestras = 20;

// Variables para frecuencia
unsigned long frecuenciaRojo = 0;
unsigned long frecuenciaVerde = 0;
unsigned long frecuenciaAzul = 0;

// Variables para min y max
unsigned long minRojo = 10000, maxRojo = 0;
unsigned long minVerde = 10000, maxVerde = 0;
unsigned long minAzul = 10000, maxAzul = 0;

void setup() {
  Serial.begin(115200);

  // Configuración pines TCS3200
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(OUT_PIN, INPUT);

  // Escala de frecuencia al 20%
  digitalWrite(S0, HIGH);
  digitalWrite(S1, LOW);

  Serial.println("Sistema de calibración iniciado...");
  delay(2000);
}

void loop() {
  Serial.println("Coloca el objeto ROJO y presiona una tecla para iniciar...");
  esperarTecla();
  medirColor(&minRojo, &maxRojo, "Rojo");

  Serial.println("\nColoca el objeto VERDE y presiona una tecla para iniciar...");
  esperarTecla();
  medirColor(&minVerde, &maxVerde, "Verde");

  Serial.println("\nColoca el objeto AZUL y presiona una tecla para iniciar...");
  esperarTecla();
  medirColor(&minAzul, &maxAzul, "Azul");

  // Mostrar resultados
  Serial.println("\nResultados Finales:");
  Serial.print("Rojo - Mínimo: "); Serial.print(minRojo); Serial.print(" Hz, Máximo: "); Serial.println(maxRojo);
  Serial.print("Verde - Mínimo: "); Serial.print(minVerde); Serial.print(" Hz, Máximo: "); Serial.println(maxVerde);
  Serial.print("Azul - Mínimo: "); Serial.print(minAzul); Serial.print(" Hz, Máximo: "); Serial.println(maxAzul);

  Serial.println("\n--- Fin del proceso ---");
  while (true); // Queda detenido
}

// --- Funciones auxiliares ---

void esperarTecla() {
  while (Serial.available()) Serial.read(); // Limpiar buffer
  while (!Serial.available()) {
    // Esperar tecla
  }
  while (Serial.available()) Serial.read(); // Limpiar de nuevo
}

unsigned long leerFrecuencia(bool s2, bool s3) {
  digitalWrite(S2, s2);
  digitalWrite(S3, s3);
  delay(100); // Pequeña pausa para estabilizar
  return pulseIn(OUT_PIN, LOW);
}

void medirColor(unsigned long* minColor, unsigned long* maxColor, const char* nombreColor) {
  *minColor = 10000;
  *maxColor = 0;

  Serial.print("Midiendo frecuencias de ");
  Serial.println(nombreColor);
  delay(500);

  for (int i = 0; i < numMuestras; i++) {
    unsigned long frecuencia = 0;

    if (strcmp(nombreColor, "Rojo") == 0) {
      frecuencia = leerFrecuencia(LOW, LOW); // Rojo
    } else if (strcmp(nombreColor, "Verde") == 0) {
      frecuencia = leerFrecuencia(HIGH, HIGH); // Verde
    } else if (strcmp(nombreColor, "Azul") == 0) {
      frecuencia = leerFrecuencia(LOW, HIGH); // Azul
    }

    Serial.print("Muestra "); Serial.print(i + 1); Serial.print(": ");
    Serial.print(frecuencia); Serial.println(" Hz");

    if (frecuencia < *minColor) *minColor = frecuencia;
    if (frecuencia > *maxColor) *maxColor = frecuencia;

    delay(200);
  }

  Serial.print("-> Min: "); Serial.print(*minColor); Serial.print(" Hz, Max: "); Serial.println(*maxColor);
}
