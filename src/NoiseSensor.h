#ifndef NOISESENSOR_H
#define NOISESENSOR_H

// Definiciones del sensor
// #define SOUNDPIN 25 // pin conectado a la señal de sonido

extern int counter;
extern bool isLow;

// Funciones
void initSensor();
void getFreqTask(void* pvParameters);

#endif // NOISESENSOR_H
