#include "Global.h"
#include <cstddef>
#include <string>

// Definição das variáveis globais

// Mutex para proteger o acesso à variável global de distância
SemaphoreHandle_t xDistanceMutex = NULL;

// Variável global para armazenar a distância medida
volatile int globalDistance = 0;

// Fator de divisão para ajustar a medida da distância
volatile int fator_divisao = 1;

// Início e fim da Zona 1
volatile int zona1_inicio = 0;
volatile int zona1_fim = 0;

// Início e fim da Zona 2
volatile int zona2_inicio = 0;
volatile int zona2_fim = 0;

// Início e fim da Zona 3
volatile int zona3_inicio = 0;
volatile int zona3_fim = 0;

// Token de autenticação do MQTT
std::string tokenMqt = "";

// variavel com ip do servidor
std::string ipServidor = "";

// variavel com porta do servidor
std::string portaServidor = "";

volatile int FiltroKalman = 0;


