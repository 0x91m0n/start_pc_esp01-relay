#define BLYNK_TEMPLATE_ID "YOUR_TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "Relay"
#define BLYNK_AUTH_TOKEN "YOUR_AUTH_TOKEN" 
#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> // Не забудь установить в менеджере библиотек

const int relayPin = 0; 
const int statusLedPin = LED_BUILTIN; 

BlynkTimer timer;
ESP8266WebServer server(80);

// Функция управления реле (универсальная)
void triggerRelay(int duration) {
  digitalWrite(relayPin, LOW); 
  Serial.print("Relay: CLOSED for "); Serial.println(duration);
  
  timer.setTimeout(duration, []() {
    digitalWrite(relayPin, HIGH); 
    Blynk.virtualWrite(V42, 0); // Сброс кнопки в приложении Blynk
    Serial.println("Relay: OPENED");
  });
}

// Легкая веб-страница управления
void handleRoot() {
  String html = "<html><head><meta charset='UTF-8'><title>PC Control</title>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>body{background:#121212;color:white;text-align:center;font-family:sans-serif;padding-top:50px;}";
  html += ".btn{background:#cc0000;border:none;color:white;padding:20px;width:85%;max-width:300px;border-radius:10px;font-size:20px;margin:10px;cursor:pointer;}";
  html += ".btn-sec{background:#444; font-size:16px;}</style></head><body>";
  html += "<h2>Управление ПК</h2>";
  html += "<a href='/press'><button class='btn'>ВКЛЮЧИТЬ (1.2с)</button></a><br>";
  html += "<a href='/hard' onclick=\"return confirm('Выключить принудительно?')\"><button class='btn btn-sec'>HARD OFF (5с)</button></a>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

// Обработка кнопки из приложения Blynk
BLYNK_WRITE(V42) {
  if (param.asInt() == 1) triggerRelay(1200);
}

void setup() {
  // Защита от случайного срабатывания при подаче питания
  pinMode(relayPin, INPUT_PULLUP);
  digitalWrite(relayPin, HIGH); 
  pinMode(relayPin, OUTPUT);
  
  Serial.begin(115200);
  pinMode(statusLedPin, OUTPUT);

  // Настройка Wi-Fi через WiFiManager
  WiFiManager wm;
  wm.setConfigPortalTimeout(180); // Ждать настройки 3 минуты
  // Создает точку доступа "PC-REMOTE-SETUP" если не нашел Wi-Fi
  if (!wm.autoConnect("PC-REMOTE-SETUP")) {
    Serial.println("Config portal timeout, restarting...");
    delay(3000);
    ESP.restart();
  }

  // Подключение к Blynk (данные берутся из дефайнов в начале)
  Blynk.config(BLYNK_AUTH_TOKEN);
  Blynk.connect();

  // Настройка сетевого имени (http://esp-01.local)
  if (MDNS.begin("esp-01")) {
    MDNS.addService("http", "tcp", 80);
  }

  // Пути веб-сервера
  server.on("/", handleRoot);
  server.on("/press", []() {
    triggerRelay(1200);
    server.sendHeader("Location", "/"); 
    server.send(303);
  });
  server.on("/hard", []() {
    triggerRelay(5000);
    server.sendHeader("Location", "/");
    server.send(303);
  });
  server.begin();

  // Настройка обновления по воздуху (OTA)
  ArduinoOTA.setHostname("esp-01");
  ArduinoOTA.setPassword("YOUR_OTA_PASSWORD"); // Придумай пароль для прошивки
  ArduinoOTA.begin();

  // Таймер проверки связи (каждые 15 сек)
  timer.setInterval(15000L, [](){
    bool connectionError = (WiFi.status() != WL_CONNECTED || !Blynk.connected());
    // Если ошибка — горит постоянно, если всё ок — погашен
    digitalWrite(statusLedPin, connectionError ? LOW : HIGH);
  });
}

void loop() {
  Blynk.run();
  ArduinoOTA.handle();
  server.handleClient();
  timer.run();
}
