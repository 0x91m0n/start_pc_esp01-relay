#define BLYNK_TEMPLATE_ID "YOUR_TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "Relay"
#define BLYNK_AUTH_TOKEN "YOUR_AUTH_TOKEN" 
#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> 

const int relayPin = 0; 
const int statusLedPin = LED_BUILTIN; 

BlynkTimer timer;
ESP8266WebServer server(80);


void triggerRelay(int duration) {
  digitalWrite(relayPin, LOW); 
  Serial.print("Relay: CLOSED for "); Serial.println(duration);
  
  timer.setTimeout(duration, []() {
    digitalWrite(relayPin, HIGH); 
    Blynk.virtualWrite(V42, 0); 
    Serial.println("Relay: OPENED");
  });
}

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


BLYNK_WRITE(V42) {
  if (param.asInt() == 1) triggerRelay(1200);
}

void setup() {
  pinMode(relayPin, INPUT_PULLUP);
  digitalWrite(relayPin, HIGH); 
  pinMode(relayPin, OUTPUT);
  
  Serial.begin(115200);
  pinMode(statusLedPin, OUTPUT);

  WiFiManager wm;
  wm.setConfigPortalTimeout(180); 
  if (!wm.autoConnect("PC-REMOTE-SETUP")) {
    Serial.println("Config portal timeout, restarting...");
    delay(3000);
    ESP.restart();
  }

  Blynk.config(BLYNK_AUTH_TOKEN);
  Blynk.connect();

  if (MDNS.begin("esp-01")) {
    MDNS.addService("http", "tcp", 80);
  }

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

  ArduinoOTA.setHostname("esp-01"); //Function for updating via Arduino IDE on the same Wi-Fi network
  ArduinoOTA.setPassword("12345678"); //default password
  ArduinoOTA.begin();

  timer.setInterval(15000L, [](){
    bool connectionError = (WiFi.status() != WL_CONNECTED || !Blynk.connected());
    digitalWrite(statusLedPin, connectionError ? LOW : HIGH);
  });
}

void loop() {
  Blynk.run();
  ArduinoOTA.handle();
  server.handleClient();
  timer.run();
}
