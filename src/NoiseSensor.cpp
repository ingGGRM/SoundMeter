#include <Arduino.h>

// Variables globales para almacenar datos
int counter = 0;

bool isLow = false;
int noisePin = 25; // pin conectado a la señal de sonido

// Inicializa el sensor DHT
void initSensor() {
  // configuracion para el sensado del sonido
  pinMode(noisePin, INPUT);
  pinMode(noisePin, INPUT_PULLUP);
  Serial.println("Sensor de ruido iniciado.");
}

// Tarea para leer el sensor DHT
void getFreqTask(void* pvParameters) {
  while (true) {
    if (!digitalRead(noisePin) && !isLow) {
      counter++;
      isLow = true;
    } else if (digitalRead(noisePin)) {
      isLow = false;
    }

    // Pausa de 2 segundos
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}
