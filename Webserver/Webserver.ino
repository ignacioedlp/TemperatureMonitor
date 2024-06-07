#include <SPIFFS.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <UniversalTelegramBot.h>
#include <time.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <DHT.h>

//WIFI
const char* ssid = "Personal-5B3-2.4GHz"; // Con tu SSID
const char* password = "9114DEA5B3"; // Tu contraseña

//Telegram
const char* telegramBotToken = "7126983538:AAEo66DMT49mDPm-Aa1_IMp3G8E_1w5cdGA";  // Token de bot de Telegram
const char* telegramChatId = "924578095";  //ID de chat de Telegram

//MQTT
//const char* mqtt_server = "a1v7zemm8o6cz2-ats.iot.sa-east-1.amazonaws.com";
//const int mqtt_port = 8883;

//DHT
#define DHTPIN 21
#define DHTTYPE DHT11
DHT dht(DHTPIN,DHTTYPE);

//Configuracion de Cliente MQTT
WiFiClientSecure espClient;
PubSubClient client(espClient);

//Configuracion BOT Telegram
WiFiClientSecure TelegramClient;
UniversalTelegramBot bot(telegramBotToken, TelegramClient);

//Certificates
String Read_rootca;
String Read_cert;
String Read_privatekey;

AsyncWebServer server(80);

//********************************

#define BUFFER_LEN  256
long lastMsg = 0;
char msg[BUFFER_LEN];
int value = 0;
byte mac[6];
char mac_Id[18];

//Api
float currentTemperatureApi = 0.0;
float previousTemperatureApi = 0.0;
float currentHumidityApi = 0.0;
float previousHumidityApi = 0.0;
//Historic
float currentTemperature = 0.0;
float previousTemperature = 0.0;
float currentHumidity = 0.0;
float previousHumidity = 0.0;

//********************************

// Función para obtener la hora actual
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

// Función de callback
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensaje recibido [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

//Conectar a broker MQTT
void reconnect() {
  
  // Loop para reconección
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    
    // Creando un ID como ramdon
    String clientId = "ESP32-";
    clientId += String(random(0xffff), HEX);
    
    // Intentando conectarse
    if (client.connect(clientId.c_str())) {
      Serial.println("conectada");
      
    // Conectado, publicando un payload...
      client.publish("ei_out", "hello world");
    
    // ... y suscribiendo
      client.subscribe("ei_in");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" Esperando 5 segundos");
      
      // Tiempo muerto de 5 segundos
      delay(5000);
    }
  }
}

void setupWifi() {
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
}

void setupCertificates() {
  // Root CA leer archivo
  File file2 = SPIFFS.open("/AmazonRootCA1.pem", "r");
  if(!file2){
    Serial.println("No se puedo abrir el archivo para leerlo");
    return;
  }

  Serial.println("Root CA File Content");
  while(file2.available()){
    Read_rootca = file2.readString();
    Serial.println(Read_rootca); 
  }

  // Cert leer archivo
  File file4 = SPIFFS.open("/a41f3-certificate.pem.crt", "r");
  if(!file4){
    Serial.println("No se puedo abrir el archivo para leerlo");
    return;
  }

  Serial.println("Cert File Content");
  while(file4.available()){
    Read_cert = file4.readString();
    Serial.println(Read_cert); 
  }

  // Private Key leer archivo
  File file6 = SPIFFS.open("/a41f3-private.pem.key", "r");
  if(!file6){
    Serial.println("No se puedo abrir el archivo para leerlo");
    return;
  }

  Serial.println("Private Key File Content");
  while(file6.available()){
    Read_privatekey = file6.readString();
    Serial.println(Read_privatekey); 
  }
}

void setup() {
  // Iniciar puerto serie
  Serial.begin(115200);

  // Iniciar DHT
  dht.begin();
  
  // Conexión a red WiFi
  setupWifi();

  // Iniciar SPIFFS
  if(!SPIFFS.begin(true)){
    Serial.println("An error has occurred while mounting SPIFFS");
    return;
  }

  // Leer certificados
  //setupCertificates();

  //=====================================================

  char* pRead_rootca;
  //pRead_rootca = (char *)malloc(sizeof(char) * (Read_rootca.length() + 1));
  //strcpy(pRead_rootca, Read_rootca.c_str());

  char* pRead_cert;
  //pRead_cert = (char *)malloc(sizeof(char) * (Read_cert.length() + 1));
  //strcpy(pRead_cert, Read_cert.c_str());

  char* pRead_privatekey;
  //pRead_privatekey = (char *)malloc(sizeof(char) * (Read_privatekey.length() + 1));
  //strcpy(pRead_privatekey, Read_privatekey.c_str());

  Serial.println("================================================================================================");
  Serial.println("Certificados que pasan adjuntan al espClient");
  Serial.println();
  Serial.println("Root CA:");
  //Serial.write(pRead_rootca);
  Serial.println("================================================================================================");
  Serial.println();
  Serial.println("Cert:");
  //Serial.write(pRead_cert);
  Serial.println("================================================================================================");
  Serial.println();
  Serial.println("privateKey:");
  //Serial.write(pRead_privatekey);
  Serial.println("================================================================================================");


  // Configurar cliente MQTT con certificados
  //espClient.setCACert(pRead_rootca);
  //espClient.setCertificate(pRead_cert);
  //espClient.setPrivateKey(pRead_privatekey);

  //client.setServer(mqtt_server, mqtt_port);
  //client.setCallback(callback);

  //******************************************

  // Obtener la dirección MAC del ESP32
  WiFi.macAddress(mac);
  snprintf(mac_Id, sizeof(mac_Id), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.println(mac_Id);

  //****************************************
  delay(2000);

  // Configuración del NTP
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  Serial.println("Waiting for NTP time sync...");
  while (!time(nullptr)) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("");
  Serial.println("Time synchronized");

  //Telegram
  TelegramClient.setInsecure(); // No verificación SSL
  bot.sendMessage(telegramChatId, "Bot iniciado", "");

  //****************************************

  // Configurar servidor web
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html");
  });

  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    previousTemperatureApi = currentTemperatureApi;
    currentTemperatureApi = dht.readTemperature();
    previousHumidityApi = currentHumidityApi;
    currentHumidityApi = dht.readHumidity();
  Serial.println("prevTemp: " + String(previousTemperatureApi));
  Serial.println("prevHumid: " + String(previousHumidityApi));
    String json = "{\"currentTemperature\": " + String(currentTemperatureApi) + ", \"temperatureDifference\": " + String(currentTemperatureApi - previousTemperatureApi) +
                  ", \"currentHumidity\": " + String(currentHumidityApi) + ", \"humidityDifference\": " + String(((currentHumidityApi - previousHumidityApi) / previousHumidityApi) * 100) + "}";
  Serial.println(json);
    request->send(200, "application/json", json);
  });

  server.begin();
}

void loop() {
  String timeStr = getTime(); 

  // Conectar al broker MQTT si no está conectado
  //if (!client.connected()) {
    //reconnect();
  //}
  //client.loop();
  previousTemperature = currentTemperature;
  currentTemperature = dht.readTemperature();
  previousHumidity = currentHumidity;
  currentHumidity = dht.readHumidity();

  if (abs(currentTemperature - previousTemperature) >= 5.0 && previousTemperature != 0.0) {
    bot.sendMessage(telegramChatId, "Alerta! La temperatura ha cambiado en: " + String(currentTemperature - previousTemperature) + " °C", "");
  }
  if (abs(((currentHumidity - previousHumidity) / previousHumidity) * 100) >= 20.0 && previousTemperature != 0.0) {
    bot.sendMessage(telegramChatId, "Alerta! La humedad ha cambiado en un: " + String(((currentHumidity - previousHumidity) / previousHumidity) * 100) + " %", "");
  }
  // Publicar mensaje cada 5 minutos
  long now = millis();
  if (now - lastMsg > 300000) {
    lastMsg = now;
    //=============================================================================================

    String macIdStr = mac_Id;
    String Temprature = String(currentTemperature);
    String Humidity = String(currentHumidity);

    snprintf (msg, BUFFER_LEN, "{\"timestamp\" : \"%s\", \"mac_id\" : \"%s\", \"temperature\" : %s, \"humidity\" : %s}", timeStr.c_str(), macIdStr.c_str(), Temprature.c_str(), Humidity.c_str());
    Serial.println(msg);
    //client.publish("sensor", msg);
    //================================================================================================
  }

}
