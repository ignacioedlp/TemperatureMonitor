#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <HTTPClient.h>
#include <time.h>

const char* ssid = "Personal-5B3-2.4GHz"; // Reemplaza con tu SSID
const char* password = "9114DEA5B3"; // Reemplaza con tu contraseña
const char* telegramBotToken = "7126983538:AAEo66DMT49mDPm-Aa1_IMp3G8E_1w5cdGA";  // Reemplaza con tu token de bot de Telegram
const char* telegramChatId = "924578095";  // Reemplaza con tu ID de chat de Telegram

AsyncWebServer server(80);

String temperatureHistory = "";
String humidityHistory = "";

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

String getTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return "N/A";
  }
  char timeStringBuff[50];
  strftime(timeStringBuff, sizeof(timeStringBuff), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(timeStringBuff);
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

  // Configuración del NTP
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  Serial.println("Waiting for NTP time sync...");
  while (!time(nullptr)) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("");
  Serial.println("Time synchronized");

  // Configurar servidor web
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html");
  });

  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    // Simular temperatura
    float simulatedTemperature = random(150, 250) / 10.0;
    float simulatedHumidity = random(150, 250) / 10.0;
    String timeStr = getTime();

    if (simulatedTemperature > 20.0) {
      sendTelegramMessage("Alerta! La temperatura ha superado los 20 grados: " + String(simulatedTemperature) + " °C");
    }

    String json = "{\"temperature\": " + String(simulatedTemperature) + ", \"humidity\": " + String(simulatedHumidity) + "}";
    temperatureHistory += timeStr + " - " + String(simulatedTemperature) + " °C<br>";
    humidityHistory += timeStr + " - " + String(simulatedHumidity) + " %<br>";

    request->send(200, "application/json", json);
  });

  server.on("/temperatureHistory", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", temperatureHistory);
  });

  server.on("/humidityHistory", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", humidityHistory);
  });

  server.begin();
}

void loop() {
  // Simular temperatura
  float simulatedTemperature = random(150, 250) / 10.0;
  float simulatedHumidity = random(150, 250) / 10.0;
  String timeStr = getTime(); 

  if (simulatedTemperature > 20.0) {
    sendTelegramMessage("Alerta! La temperatura ha superado los 20 grados: " + String(simulatedTemperature) + " °C");
  }

  temperatureHistory += timeStr + " - " + String(simulatedTemperature) + " °C<br>";
  humidityHistory += timeStr + " - " + String(simulatedHumidity) + " %<br>";

  delay(300000); // Espera 5 minutos
}
