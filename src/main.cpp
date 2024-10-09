#include <Wire.h>
#include <HardwareSerial.h>
#include "Arduino.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include <string.h>
#include <math.h>

#include <Preferences.h>
#include "WiFiManager.h"
#include "ConfigHandler.h"

#include "LIDARLite.h"

#include "Global.h"

// Inicialização de variáveis globais e defines

// Inicialização do objeto LIDARLite
LIDARLite lidarLite;

// Variáveis para o filtro de Kalman
float Q = 0.001; // Variância do processo (aumentado para maior responsividade)
float R = 0.5;   // Variância da medição (ajustado para maior responsividade)
float P = 1, K;
float filteredDistance = 0;
/*
    Filtro de Kalman
    ----------------
    O filtro de Kalman é um método estatístico avançado que estima o estado de um sistema dinâmico a partir de medições ruidosas.
    Implementação:
    - Requer a definição de modelos de previsão e atualização.
    - A implementação envolve cálculos de variância e aplicação de matrizes.
*/
void lidarTask(void *pvParameters)
{
  // Inicialização do LIDARLite
  lidarLite.begin(0, true);

  // Define a primeira leitura como o valor inicial filtrado
  filteredDistance = lidarLite.distance();

  while (1)
  {
    int distance = lidarLite.distance(); // Faz uma leitura da distância

    // Previsão do valor futuro
    P = P + Q;

    // Atualização do filtro de Kalman
    K = P / (P + R);
    filteredDistance = filteredDistance + K * (distance - filteredDistance);
    P = (1 - K) * P;

    // Proteção para o acesso à variável global
    if (xSemaphoreTake(xDistanceMutex, portMAX_DELAY) == pdTRUE)
    {
      globalDistance = (int)filteredDistance;
      xSemaphoreGive(xDistanceMutex);
    }

    // Aguarda 10 milissegundos antes da próxima medição (reduzido para aumentar a frequência)
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

// Função de configuração inicial do ESP32-S3
void setup()
{
  // Inicializa a comunicação serial para depuração
  Serial.begin(115200);

  // Inicializa a memória não volátil
  preferences.begin("config", false);

  if (!preferences.getBool("configSalva", false)) {
    printf("Configurações não salvas. Resetando para valores padrão.\n");
    resetarConfiguracoes(preferences);
    
    String ssid = preferences.getString("ssid");
    String senha = preferences.getString("senha");

    printf("SSID: %s" , ssid.c_str());
    printf("Senha: %s" , senha.c_str());
  }

  // Configura o servidor para salvar e recuperar configurações
  setupConfigHandler(server, preferences);

  // Configura e conecta ao WiFi
  setupWiFi();

  // Cria o mutex para sincronização de acesso à variável de distância
  xDistanceMutex = xSemaphoreCreateMutex();

  // Verifica se o mutex foi criado com sucesso
  if (xDistanceMutex == NULL)
  {
    Serial.println("Erro ao criar o mutex");
    while (1)
      ; // Para o programa em caso de falha
  }

  // Cria a tarefa para lidar com o LIDAR-Lite
  xTaskCreate(
      lidarTask,    // Função da tarefa
      "Lidar Task", // Nome da tarefa
      4096,         // Tamanho da stack alocada para a tarefa
      NULL,         // Parâmetros passados para a tarefa (neste caso, nenhum)
      1,            // Prioridade da tarefa
      NULL);        // Referência da tarefa criada

  Serial.println("Inicializando LiDAR-Lite v3HP...");
}

int blink = 0;
int lastDistance = -1;
void loop()
{
  int distanceCopy;

  // Protege o acesso à variável global de distância
  if (xSemaphoreTake(xDistanceMutex, portMAX_DELAY) == pdTRUE)
  {
    distanceCopy = globalDistance;
    xSemaphoreGive(xDistanceMutex);
  }

  // Exibe a distância medida no terminal serial apenas se houver uma mudança significativa
  if (abs(distanceCopy - lastDistance) >= 2)
  {
    Serial.print("Distância medida: ");
    Serial.println(distanceCopy);
    lastDistance = distanceCopy;
  }

  delay(100); // Aguarda 100 ms antes de repetir a leitura (reduzido para maior responsividade)
}