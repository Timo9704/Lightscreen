#include <Wire.h>
#include <FastLED.h>
#include <WiFiClient.h>
#include <WiFi.h>
#include <WebServer.h>
#include <RtcDS3231.h>
#include <time.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <LedStrip.h>


RtcDS3231<TwoWire> rtc(Wire);



//variables for Clock over Internet
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

int wifiCheck = 0;

AsyncWebServer server(80);

int hour = 0;
int minute = 0;
int second = 0;

long loopTime;



LedStrip* ledsTop = new LedStrip(17, "top");
LedStrip* ledsBottom = new LedStrip(16, "bottom");



void setup() {
  
  Serial.begin(9600);

  if(!SPIFFS.begin()){
     Serial.println("An Error has occurred while mounting SPIFFS");
     return;
  }

  FastLED.addLeds<NEOPIXEL, 17>(ledsTop->getLedCRGB(),NUM_LEDS);
  FastLED.addLeds<NEOPIXEL, 16>(ledsBottom->getLedCRGB(),NUM_LEDS);

  FastLED.clear();
  FastLED.show();

  ledsTop->setColor(150,255,255);
  ledsBottom->setColor(185,0,255);

  ledsTop->setIntervallMillis();
  ledsBottom->setIntervallMillis();

  WiFi.mode(WIFI_STA);
  WiFi.begin("SSID", "Passwort");
  Serial.println("Verbindung wird hergestellt");
  while (WiFi.status() != WL_CONNECTED) {
    delay(5000);
    if (WiFi.status() != WL_CONNECTED) {
      ESP.restart();
    }

  }
  Serial.println();
  Serial.print("Verbunden! IP:");
  Serial.println(WiFi.localIP());

  server.onNotFound([](AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Link wurde nicht gefunden!");
  });

  server.on("/setClock", [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", printLocalTime());
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    printLocalTime();
  });

  server.on("/on", [](AsyncWebServerRequest *request) {
    ledsTop->autoMode = 0;
    ledsBottom->autoMode = 0;
    ledsTop->setLedsBrightness(ledsTop->maxBrightness);
    ledsBottom->setLedsBrightness(ledsTop->maxBrightness);
    FastLED.show();
    request->send(200, "text/html", "ON");
  });

  server.on("/off", [](AsyncWebServerRequest *request) {
    ledsTop->autoMode = 0;
    ledsBottom->autoMode = 0;
    ledsTop->setLedsBrightness(0);
    ledsBottom->setLedsBrightness(0);
    FastLED.show();
    request->send(200, "text/html", "OFF");
  });

  server.on("/setFadeTime", [](AsyncWebServerRequest *request) {
    AsyncWebParameter* p1 = request->getParam(0);
    AsyncWebParameter* p2 = request->getParam(1);
    AsyncWebParameter* p3 = request->getParam(2);
    AsyncWebParameter* p4 = request->getParam(3);
    
    if ((String) p1->value() == "top"){
      ledsTop->setFadeTime(p2->value(), p3->value().toInt(), p4->value().toInt());
    }else if((String) p1->value() == "bottom"){
      ledsBottom->setFadeTime(p2->value(), p3->value().toInt(), p4->value().toInt());
    }
    request->send(200, "text/html", "Zeit wurde gesetzt!");
  });
  
  server.on("/stripBrightness", [](AsyncWebServerRequest *request) {
    AsyncWebParameter* p1 = request->getParam(0);
    AsyncWebParameter* p2 = request->getParam(1);
    
    if ((String) p1->value() == "top"){
      ledsTop->setLedsBrightness(p2->value().toInt());
    }else if((String) p1->value() == "bottom"){
      ledsBottom->setLedsBrightness(p2->value().toInt());
    }
    request->send(200, "text/html", "");
  });

  server.on("/nightMode", [](AsyncWebServerRequest *request) {
    //nightMode(server.arg(0).toInt());
    request->send(200, "text/html", "NightMode aktiviert!");
  });

  server.on("/getInformation", [](AsyncWebServerRequest *request) {
    String json = "\"ledStrips\":[" + ledsTop->exportAsJson();
    json += "," + ledsBottom->exportAsJson() + "]";
  
    request->send(200, "text/html", json);
  });

  server.on("/configPage", [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/configPage.html", "text/html");
  });

  server.on("/setColor", [](AsyncWebServerRequest *request) {
    AsyncWebParameter* p1 = request->getParam(0);
    AsyncWebParameter* p2 = request->getParam(1);
    AsyncWebParameter* p3 = request->getParam(2);

    if ((String) p1->value() == "top"){
      ledsTop->setColor(p2->value().toInt(), p3->value().toInt(), ledsTop->currentBrightness);
    }else if((String) p1->value() == "bottom"){
      ledsBottom->setColor(p2->value().toInt(), p3->value().toInt(), ledsBottom->currentBrightness);
    }
    request->send(200, "text/html", "Farbe wurde gesetzt!");
  });

  rtc.Begin();

  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  rtc.SetDateTime(compiled);
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();

  server.begin();
}


void loop() {
  loopTime = millis();
  wifiCheck++;

  if (WiFi.status() != WL_CONNECTED && wifiCheck > 80000) {
      ESP.restart();
      wifiCheck = 0;
  };

  //server.handleClient();

  RtcDateTime now = rtc.GetDateTime();
  hour = now.Hour();
  minute = now.Minute();

  int time = hour * HOUR + minute * MINUTE;

  ledsTop->fadeBrightness(time, loopTime, ledsTop->getCurrentBrightness());
  ledsBottom->fadeBrightness(time, loopTime, ledsBottom->getCurrentBrightness());
  FastLED.show();

}

//Print/set the local time in RTC from NTP
String printLocalTime(){
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");

  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  RtcDateTime compiled = RtcDateTime(timeinfo.tm_year, timeinfo.tm_mon, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
  rtc.SetDateTime(compiled);
  RtcDateTime now = rtc.GetDateTime();
 
  return now.Hour() + ":" + now.Minute();
}
