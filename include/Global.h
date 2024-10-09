#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

// Declaração das variáveis globais como `extern` para serem usadas em outros módulos

// Mutex para proteger o acesso à variável global de distância
extern SemaphoreHandle_t xDistanceMutex;

// Variável global para armazenar a distância medida
extern volatile int globalDistance;

#endif // GLOBALS_H
