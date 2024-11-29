#include "DHTSensor.h"
#include <Arduino.h>

DHT dht(DHTPIN, DHTTYPE);

// Variables globales para almacenar datos
float temperature = 0;
float humidity = 0;
bool refresh = false;

// Inicializa el sensor DHT
void initDHT() {
  dht.begin();
  Serial.println("DHT Sensor iniciado.");
}

// Tarea para leer el sensor DHT
void readDHTTask(void* pvParameters) {
  while (true) {
    humidity = dht.readHumidity();
    temperature = dht.readTemperature();

    if (isnan(humidity) || isnan(temperature)) {
      Serial.println("Error al leer el sensor DHT. Intentando nuevamente...");
    } else {
      Serial.printf("Humedad: %.2f%% | Temperatura: %.2f°C\n", humidity, temperature);
    }

    refresh = true;

    // Pausa de 2 segundos
    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }
}
