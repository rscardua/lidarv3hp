// ConfigHandler.cpp
#include "ConfigHandler.h"
#include <ESPAsyncWebServer.h>
#include <Preferences.h>
#include <ArduinoJson.h>

void salvarConfiguracoes(AsyncWebServerRequest *request, Preferences &preferences) {
  String ssid = request->getParam("ssid", true)->value();
  String senha = request->getParam("senha", true)->value();
  String ipServidor = request->getParam("ip-servidor", true)->value();
  String portaServidor = request->getParam("porta-servidor", true)->value();
  String token = request->getParam("token", true)->value();
  String inicioZona1 = request->getParam("inicio-zona-1", true)->value();
  String fimZona1 = request->getParam("fim-zona-1", true)->value();
  String inicioZona2 = request->getParam("inicio-zona-2", true)->value();
  String fimZona2 = request->getParam("fim-zona-2", true)->value();
  String inicioZona3 = request->getParam("inicio-zona-3", true)->value();
  String fimZona3 = request->getParam("fim-zona-3", true)->value();

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

  request->send(200, "text/plain", "Configurações salvas com sucesso!");
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
  // Servidor para salvar as configurações
  server.on("/salvar", HTTP_POST, [&preferences](AsyncWebServerRequest *request) {
    salvarConfiguracoes(request, preferences);
  });

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
