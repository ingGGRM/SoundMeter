#ifndef DHTSENSOR_H
#define DHTSENSOR_H

#include <Adafruit_Sensor.h>
#include <DHT.h>

// Definiciones del sensor
#define DHTPIN 27     // Pin conectado al sensor DHT
#define DHTTYPE DHT22 // Tipo de sensor

extern float temperature;
extern float humidity;
extern bool refresh;

// Funciones
void initDHT();
void readDHTTask(void* pvParameters);

#endif // DHTSENSOR_H
