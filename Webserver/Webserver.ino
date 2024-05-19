#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <HTTPClient.h>

const char* ssid = "Personal-5B3-2.4GHz"; // Reemplaza con tu SSID
const char* password = "9114DEA5B3"; // Reemplaza con tu contraseña
const char* telegramBotToken = "7126983538:AAEo66DMT49mDPm-Aa1_IMp3G8E_1w5cdGA";  // Reemplaza con tu token de bot de Telegram
const char* telegramChatId = "924578095";  // Reemplaza con tu ID de chat de Telegram

AsyncWebServer server(80);

String temperatureHistory = "";

void sendTelegramMessage(String message) {
  HTTPClient http;
  String url = "https://api.telegram.org/bot";
  url += telegramBotToken;
  url += "/sendMessage?chat_id=";
  url += telegramChatId;
  url += "&text=";
  url += message;
  
  http.begin(url);
  int httpCode = http.GET();
  if (httpCode > 0) {
    String payload = http.getString();
    Serial.println(payload);
  } else {
    Serial.println("Error on sending POST: " + String(http.errorToString(httpCode).c_str()));
  }
  http.end();
}

void setup() {
  Serial.begin(115200);

  // Conectar a Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  
  // Imprimir la dirección IP
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Iniciar SPIFFS
  if(!SPIFFS.begin(true)){
    Serial.println("An error has occurred while mounting SPIFFS");
    return;
  }

  // Configurar servidor web
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html");
  });

  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    // Simular temperatura
    float simulatedTemperature = random(150, 250) / 10.0;
    if (simulatedTemperature > 20.0) {
      sendTelegramMessage("Alerta! La temperatura ha superado los 20 grados: " + String(simulatedTemperature) + " °C");
    }
    temperatureHistory += String(simulatedTemperature) + "<br>";
    request->send(200, "text/plain", String(simulatedTemperature));
  });

  server.on("/history", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", temperatureHistory);
  });

  server.begin();
}

void loop() {
  // Simular temperatura
  float simulatedTemperature = random(150, 250) / 10.0;
  if (simulatedTemperature > 20.0) {
    sendTelegramMessage("Alerta! La temperatura ha superado los 20 grados: " + String(simulatedTemperature) + " °C");
  }
  temperatureHistory += String(simulatedTemperature) + "<br>";
  delay(300000); // Espera 5 minutos
}
