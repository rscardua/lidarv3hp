// ConfigHandler.cpp
#include "ConfigHandler.h"
#include <ESPAsyncWebServer.h>
#include <Preferences.h>
#include <ArduinoJson.h>

/**
 * @brief Salva as configurações recebidas via requisição HTTP na memória não volátil (NVS).
 *
 * Esta função recebe uma requisição HTTP contendo um JSON com as configurações e as salva na memória não volátil.
 * Se algum parâmetro necessário estiver ausente ou se ocorrer um erro na deserialização do JSON, uma resposta de erro é enviada.
 *
 * @param request Ponteiro para o objeto AsyncWebServerRequest que representa a requisição HTTP.
 * @param preferences Referência para o objeto Preferences utilizado para acessar a memória não volátil.
 * @param data Ponteiro para os dados recebidos na requisição HTTP.
 * @param len Tamanho dos dados recebidos.
 */
void salvarConfiguracoes(AsyncWebServerRequest *request, Preferences &preferences, uint8_t *data, size_t len) {
  // Converte os dados recebidos em uma string
  String jsonString = String((char*)data, len);

  // Cria um objeto JSON para armazenar os dados
  StaticJsonDocument<512> jsonDoc; // Ajuste o tamanho conforme necessário

  // Deserializa a string JSON
  DeserializationError error = deserializeJson(jsonDoc, jsonString);

  if (error) {
    Serial.print(F("Erro ao deserializar JSON: "));
    Serial.println(error.f_str());
    request->send(400, "application/json", "{\"error\":\"JSON inválido\"}");
    return;
  }

  // Extrai os valores do JSON
  String ssid = jsonDoc["ssid"] | "";
  String senha = jsonDoc["senha"] | "";
  String ipServidor = jsonDoc["ip-servidor"] | "";
  String portaServidor = jsonDoc["porta-servidor"] | "";
  String token = jsonDoc["token"] | "";
  String inicioZona1 = jsonDoc["inicio-zona-1"] | "";
  String fimZona1 = jsonDoc["fim-zona-1"] | "";
  String inicioZona2 = jsonDoc["inicio-zona-2"] | "";
  String fimZona2 = jsonDoc["fim-zona-2"] | "";
  String inicioZona3 = jsonDoc["inicio-zona-3"] | "";
  String fimZona3 = jsonDoc["fim-zona-3"] | "";

  // Verifica se todos os campos necessários foram fornecidos
  //if (ssid == "" || senha == "" || ipServidor == "" || portaServidor == "" || token == "" ||
  //    inicioZona1 == "" || fimZona1 == "" || inicioZona2 == "" || fimZona2 == "" || inicioZona3 == "" || fimZona3 == "") {
  //  request->send(400, "application/json", "{\"error\":\"Parâmetros ausentes\"}");
  //  return;
  //}

  // Salva as informações na memória não volátil (NVS)
  preferences.putString("ssid", ssid);
  preferences.putString("senha", senha);
  preferences.putString("ipServidor", ipServidor);
  preferences.putString("portaServidor", portaServidor);
  preferences.putString("token", token);
  preferences.putString("inicioZona1", inicioZona1);
  preferences.putString("fimZona1", fimZona1);
  preferences.putString("inicioZona2", inicioZona2);
  preferences.putString("fimZona2", fimZona2);
  preferences.putString("inicioZona3", inicioZona3);
  preferences.putString("fimZona3", fimZona3);

  // Define a flag indicando que a configuração foi salva
  preferences.putBool("configSalva", true);

  // Envia uma resposta de sucesso
  request->send(200, "application/json", "{\"message\":\"Configurações salvas com sucesso!\"}");
}


void recuperarConfiguracoes(AsyncWebServerRequest *request, Preferences &preferences) {
  if (!preferences.getBool("configSalva", false)) {
    resetarConfiguracoes(preferences);
  }

  JsonDocument jsonResponse;
  
  jsonResponse["ssid"] = preferences.getString("ssid", "CLARO_D4D094");
  jsonResponse["senha"] = preferences.getString("senha", "NYJmv24gGv");
  jsonResponse["ipServidor"] = preferences.getString("ipServidor", "192.168.1.1");
  jsonResponse["portaServidor"] = preferences.getString("portaServidor", "8080");
  jsonResponse["token"] = preferences.getString("token", "default_token");
  jsonResponse["inicioZona1"] = preferences.getString("inicioZona1", "10");
  jsonResponse["fimZona1"] = preferences.getString("fimZona1", "20");
  jsonResponse["inicioZona2"] = preferences.getString("inicioZona2", "20");
  jsonResponse["fimZona2"] = preferences.getString("fimZona2", "30");
  jsonResponse["inicioZona3"] = preferences.getString("inicioZona3", "30");
  jsonResponse["fimZona3"] = preferences.getString("fimZona3", "40");

  String response;
  serializeJson(jsonResponse, response);
  request->send(200, "application/json", response);
}

void resetarConfiguracoes(Preferences &preferences) {
  preferences.putString("ssid", "CLARO_D4D094");
  preferences.putString("senha", "NYJmv24gGv");
  preferences.putString("ipServidor", "192.168.1.1");
  preferences.putString("portaServidor", "8080");
  preferences.putString("token", "default_token");
  preferences.putString("inicioZona1", "10");
  preferences.putString("fimZona1", "20");
  preferences.putString("inicioZona2", "20");
  preferences.putString("fimZona2", "30");
  preferences.putString("inicioZona3", "30");
  preferences.putString("fimZona3", "40");

  // Define a flag indicando que a configuração não foi salva
  preferences.putBool("configSalva", false);
}

void setupConfigHandler(AsyncWebServer &server, Preferences &preferences) {
  // Define a rota para requisições POST em "/config"
  // Define a rota para requisições POST em "/salvar"
  server.on("/salvar", HTTP_POST, 
    // requestHandler - chamado quando a requisição é recebida
    [](AsyncWebServerRequest *request){
      Serial.println("Recebida requisição POST em /salvar");
      // A resposta será enviada no bodyHandler
    }, 
    // uploadHandler - não utilizado neste caso
    NULL, 
    // bodyHandler - chamado quando o corpo da requisição é recebido
    [&preferences](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      Serial.printf("Recebendo dados: %u bytes\n", len);
      salvarConfiguracoes(request, preferences, data, len);
    }
  );


  // Servidor para salvar as configurações
  //server.on("/salvar", HTTP_POST, NULL, NULL, [&preferences](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
  //  salvarConfiguracoes(request, preferences, data, len);
  //});;

  // Servidor para recuperar as configurações
  server.on("/recuperar", HTTP_GET, [&preferences](AsyncWebServerRequest *request) {
    recuperarConfiguracoes(request, preferences);
  });

  // Servidor para resetar as configurações para os valores padrão
  server.on("/resetar", HTTP_POST, [&preferences](AsyncWebServerRequest *request) {
    resetarConfiguracoes(preferences);
    request->send(200, "text/plain", "Configurações resetadas para os valores padrão com sucesso!");
  });
}
