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

// Fator de divisão para ajustar a leitura da distância
extern volatile int fator_divisao;

// Início e fim da Zona 1
extern volatile int zona1_inicio;
extern volatile int zona1_fim;

// Início e fim da Zona 2
extern volatile int zona2_inicio;
extern volatile int zona2_fim;

// Início e fim da Zona 3
extern volatile int zona3_inicio;
extern volatile int zona3_fim;
// Token de autenticação do MQTT
extern std::string tokenMqt;

// variavel com ip do servidor
extern std::string ipServidor;

// variavel com porta do servidor
extern std::string portaServidor;


#endif // GLOBALS_H
