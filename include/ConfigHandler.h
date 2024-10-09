// config_handler.h
#ifndef CONFIG_HANDLER_H
#define CONFIG_HANDLER_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>

/**
 * @brief Salva as configurações recebidas na memória não volátil (NVS).
 * 
 * @param request Ponteiro para o objeto AsyncWebServerRequest contendo os dados da requisição.
 * @param preferences Referência ao objeto Preferences utilizado para armazenar os dados na NVS.
 */
void salvarConfiguracoes(AsyncWebServerRequest *request, Preferences &preferences);

/**
 * @brief Recupera as configurações salvas da memória não volátil (NVS).
 * 
 * @param request Ponteiro para o objeto AsyncWebServerRequest contendo os dados da requisição.
 * @param preferences Referência ao objeto Preferences utilizado para recuperar os dados da NVS.
 */
void recuperarConfiguracoes(AsyncWebServerRequest *request, Preferences &preferences);

/**
 * @brief Reseta as configurações para os valores padrão.
 * 
 * @param preferences Referência ao objeto Preferences utilizado para resetar os dados na NVS.
 */
void resetarConfiguracoes(Preferences &preferences);

/**
 * @brief Configura as rotas do servidor para manipulação das configurações.
 * 
 * @param server Referência ao objeto AsyncWebServer utilizado para configurar as rotas.
 * @param preferences Referência ao objeto Preferences utilizado para manipular os dados da NVS.
 */
void setupConfigHandler(AsyncWebServer &server, Preferences &preferences);

#endif