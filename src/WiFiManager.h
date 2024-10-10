#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <Preferences.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>

#include "Global.h"

Preferences preferences;
AsyncWebServer server(80);

/**
 * @brief Inicia o modo Access Point para configuração do ESP32.
 *
 * Esta função configura o ESP32 para operar no modo Access Point (AP), permitindo que dispositivos se conectem a ele para realizar configurações via uma interface web.
 *
 * Funcionalidades:
 * - Inicia o modo AP com o nome de rede "ESP32-Config".
 * - Exibe no console o nome da rede e o IP do AP.
 * - Define várias rotas HTTP para servir páginas HTML e processar requisições de configuração e login.
 * - Realiza a varredura de redes Wi-Fi disponíveis e exibe uma lista dessas redes.
 * - Permite a conexão a uma rede Wi-Fi selecionada.
 * @note Esta função deve ser chamada para iniciar o modo AP e configurar as rotas HTTP.
 */
void startAccessPoint()
{
  // Inicia o modo Access Point para configuração
  String apName = "ESP32-Config";
  WiFi.softAP(apName.c_str());

  Serial.print("Modo AP iniciado. Nome da rede: ");
  Serial.println(apName);
  Serial.print("IP do AP: ");
  Serial.println(WiFi.softAPIP());
}

/**
 * @brief Configura a conexão WiFi do dispositivo.
 *
 * Esta função inicializa o sistema de arquivos SPIFFS, recupera as credenciais de WiFi
 * armazenadas na memória não volátil (NVS), e tenta conectar ao WiFi usando essas credenciais.
 * Se não houver credenciais salvas, ou se a conexão falhar, o dispositivo entra em modo Access Point (AP)
 * para permitir a configuração das credenciais de WiFi.
 */
void setupWiFi()
{
  // Inicializa o sistema de arquivos SPIFFS
  if (!SPIFFS.begin(true))
  {
    Serial.println("Erro ao montar SPIFFS");
    return;
  }

  // Recupera SSID e senha da memória não volátil (NVS)
  String ssid = "CLARO_2GD4D094"; // preferences.getString("ssid", "");
  String senha = preferences.getString("senha", "");

  // Imprime SSID e senha recuperados
  Serial.printf("SSID: %s", ssid.c_str());
  Serial.printf("Senha: %s", senha.c_str());

  // Habilita CORS (Cross-Origin Resource Sharing)
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET, POST, PUT");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Content-Type");

  // Verifica se há credenciais salvas
  if (ssid.isEmpty())
  {
    Serial.println("Nenhuma configuração de WiFi salva. Entrando em modo AP para configuração.");
    startAccessPoint();
    return;
  }

  // Conecta ao WiFi usando as credenciais armazenadas
  WiFi.begin(ssid.c_str(), senha.c_str());

  Serial.print("Conectando-se a ");
  Serial.println(ssid);

  // Aguarda conexão com limite de tempo
  int tentativas = 0;
  while (WiFi.status() != WL_CONNECTED && tentativas < 10)
  { // Limite de 30 tentativas (~30 segundos)
    delay(1000);
    Serial.print(".");
    tentativas++;
  }

  // Verifica se a conexão foi bem-sucedida
  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("");
    Serial.print("Conectado ao WiFi: ");
    Serial.println(WiFi.localIP());
  }
  else
  {
    Serial.println("");
    Serial.println("Falha ao conectar ao WiFi. Verifique o SSID e a senha e tente novamente.");
    // Limpa as credenciais salvas para evitar novas tentativas com dados incorretos
    preferences.remove("ssid");
    preferences.remove("senha");
    Serial.println("Credenciais de WiFi removidas. Entrando em modo AP para configuração.");
    startAccessPoint();
  }

  // Define a rota para servir a página home.html do sistema de arquivos SPIFFS
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/home.html", "text/html"); });

  // Define a rota para configuração via web
  server.on("/configurar", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/config.html", "text/html"); });

  // Define a rota para login via web
  server.on("/login", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/login.html", "text/html"); });

  // Define a rota para validar credenciais
  server.on("/validarCredenciais", HTTP_POST, [](AsyncWebServerRequest *request)
            {
    if (request->hasParam("username", true) && request->hasParam("password", true)) {
      String username = request->getParam("username", true)->value();
      String password = request->getParam("password", true)->value();

      // Obtém a senha salva nas preferências
      String passwordSalva = preferences.getString("LoginPassword", "admin");
      String usernameSalvo = preferences.getString("LoginUserName", "admin");
 
      // se senha e password forem vazias, assume um valor padrão
      if (passwordSalva.isEmpty()) {
        passwordSalva = "admin";
        usernameSalvo = "admin"; // TODO: tenho que implementar um hash para a senha
      }

      Serial.printf("Username: %s\n", username.c_str());
      Serial.printf("Senha: %s\n", password.c_str());
      Serial.println();
      Serial.printf("Username salvo: %s\n", usernameSalvo.c_str());
      Serial.printf("Senha salva: %s\n", passwordSalva.c_str());


      if (password == passwordSalva && username == usernameSalvo) {
        String response = "{\"success\": true}";
        request->send(200, "application/json", response);
      } else {
        String response = "{\"success\": false}";
        request->send(200, "application/json", response);
      }
    } else {
      request->send(400, "application/json", "{\"error\": \"Parâmetros inválidos\"}");
    } });

  // Configura o servidor para responder à rota principal
  server.on("/getWifi", HTTP_GET, [](AsyncWebServerRequest *request)
            {
        String htmlPage = "<html><head><title>Lista de WiFi</title></head><body>";
        htmlPage += "<h1>Redes Wi-Fi Disponíveis</h1>";
        htmlPage += "<ul>";

        // Realiza a varredura de redes Wi-Fi disponíveis
        int numberOfNetworks = WiFi.scanNetworks();
        for (int i = 0; i < numberOfNetworks; i++) {
            htmlPage += "<li>";
            htmlPage += "<a href=\"/connect?ssid=" + WiFi.SSID(i) + "\">";
            htmlPage += WiFi.SSID(i);
            htmlPage += "</a> (Sinal: ";
            htmlPage += WiFi.RSSI(i);
            htmlPage += " dBm)";
            htmlPage += "</li>";
        }
        htmlPage += "</ul>";
        htmlPage += "</body></html>";

        // Envia a página HTML como resposta
        request->send(200, "text/html", htmlPage); });

  // Configura o servidor para responder à rota de conexão
  server.on("/connect", HTTP_GET, [](AsyncWebServerRequest *request)
            {
        if (request->hasParam("ssid")) {
            String ssid = request->getParam("ssid")->value();
            String htmlPage = "<html><head><title>Conectar ao WiFi</title></head><body>";
            htmlPage += "<h1>Conectar à Rede Wi-Fi: " + ssid + "</h1>";
            htmlPage += "<form action=\"/connectToWiFi\" method=\"POST\">";
            htmlPage += "<input type=\"hidden\" name=\"ssid\" value=\"" + ssid + "\">";
            htmlPage += "<label for=\"password\">Senha:</label>";
            htmlPage += "<input type=\"password\" id=\"password\" name=\"password\"><br><br>";
            htmlPage += "<input type=\"submit\" value=\"Conectar\">";
            htmlPage += "</form>";
            htmlPage += "</body></html>";

            // Envia a página HTML como resposta
            request->send(200, "text/html", htmlPage);
        } else {
            request->send(400, "text/plain", "SSID não fornecido");
        } });

  // Configura o servidor para responder à rota de conexão com a rede Wi-Fi
  server.on("/connectToWiFi", HTTP_POST, [](AsyncWebServerRequest *request)
            {
        if (request->hasParam("ssid", true) && request->hasParam("password", true)) {
            String ssid = request->getParam("ssid", true)->value();
            String password = request->getParam("password", true)->value();

            WiFi.begin(ssid.c_str(), password.c_str());

            String htmlPage = "<html><head><title>Conectar ao WiFi</title></head><body>";
            htmlPage += "<h1>Conectando à Rede Wi-Fi: " + ssid + "</h1>";
            htmlPage += "<p>Aguarde enquanto tentamos conectar...</p>";
            htmlPage += "</body></html>";

            // Envia a página HTML como resposta
            request->send(200, "text/html", htmlPage);
        } else {
            request->send(400, "text/plain", "Dados insuficientes para conectar");
        } });

  // Rota que retorna o valor do sensor
  server.on("/getDistance", HTTP_GET, [](AsyncWebServerRequest *request)
            {
        // Protege o acesso à variável global de distância
        int distanceCopy = 0;
        if (xSemaphoreTake(xDistanceMutex, portMAX_DELAY) == pdTRUE)
        {
          distanceCopy = globalDistance;
          xSemaphoreGive(xDistanceMutex);
        }        
        request->send(200, "text/plain", String(distanceCopy)); });

  // Define a rota para servir arquivos estáticos, como CSS e JavaScript
  server.serveStatic("/styles.css", SPIFFS, "/styles.css").setCacheControl("max-age=600");
  server.serveStatic("/Roboto.woff2", SPIFFS, "/Roboto.woff2").setCacheControl("max-age=600");

  server.begin();
}

#endif