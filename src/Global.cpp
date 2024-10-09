#include "Global.h"
#include <cstddef>

// Definição das variáveis globais

// Mutex para proteger o acesso à variável global de distância
SemaphoreHandle_t xDistanceMutex = NULL;

// Variável global para armazenar a distância medida
volatile int globalDistance = 0;
