#include <Wire.h>
#include <FastLED.h>
#include <WiFiClient.h>
#include <WiFi.h>
#include <WebServer.h>
#include <RtcDS3231.h>
#include <time.h>

RtcDS3231<TwoWire> rtc(Wire);

//set number of used LEDs per strip in NUM_LEDS 
#define NUM_LEDS 40
#define HOUR 3600
#define MINUTE 60

//variables for Clock over Internet
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

int wifiCheck = 0;

WebServer server(80);

int hour = 0;
int minute = 0;
int second = 0;

long loopTime;

class LedStrip : CFastLED {
  public:
  bool fadeUp = false;
  bool fadeDown = false;
  bool autoMode = true;
  bool on = false;
  bool off = false;
  int upTime = 12 * HOUR + 2 * MINUTE;;
  int downTime = 17 * HOUR + 00 * MINUTE;;
  int currentBrightness = 0;
  int maxBrightness = 255;
  int h = 255;
  int s = 255;
  int v = 255;
  int intervallDuration = 2;
  long intervallMillis;
  int pin;
  String identifier;
  CRGB leds[NUM_LEDS]; 

  LedStrip(uint8_t pin, String identifier){
    this->pin = pin;
    this->identifier = identifier;
  }

  void setIntervallMillis(){
    this->intervallMillis = millis();
  }

  void setAutoMode(int state){
    if(state == 0){
      autoMode = 0;
    }else if(state == 1){
      autoMode = 1;
    }
  }

  int getCurrentBrightness(){
    return this->currentBrightness;
  }

  int setCurrentBrightness(int brightness){
    this->currentBrightness = brightness;
  }

  CRGB* getLedCRGB(){
    return this->leds;
  }

  void setLedsBrightness(int newBrightness) {
    Serial.print("CurrentBrightness: ");
    Serial.print(getCurrentBrightness());
    Serial.print(" / NewBrightness: ");
    Serial.print(newBrightness);

    CHSV color = CHSV(h, s, newBrightness);

    if (newBrightness != getCurrentBrightness()){
      fill_solid(leds, NUM_LEDS, color);
      this->currentBrightness = newBrightness;
    }
  }

  void nightMode(int dauer) {
    for (int i = 0; i < NUM_LEDS; i++) {
      if ((i % 2) == 0) {
        CHSV color = CHSV(h, s, 100);
        fill_solid(leds, NUM_LEDS, color);
      } else {
        CHSV color = CHSV(h, s, 0);
        fill_solid(leds, NUM_LEDS, color);
      }
  }
}

  void setColor(int h, int s, int v) {
    if (h <= 255 && s <= 255 && v <= 255) {
      this->h = h;
      this->s = s;
      this->v = v;
      
      CHSV color = CHSV(h, s, v);

      fill_solid(leds, NUM_LEDS, color);
    }
    FastLED.show();
  }

  int timeCheck(int time){
   
    if(time == upTime){
      fadeUp = true;
      off = false;
    }
    if(time == downTime){
      fadeDown = true;
      on = false;
    }
    if(time > upTime && time < downTime && fadeUp == false){
      on = true;
    }
    if(time < upTime || time > downTime && fadeDown == false){
      off = true;
    }
    
}

  int fadeBrightness(int time, long loopTime, int currentBrightness){
    timeCheck(time);

    unsigned long fadeCycleTime = intervallMillis + intervallDuration * 1000;
    
  
    if(loopTime >= fadeCycleTime && autoMode){
      setIntervallMillis();
      if(fadeUp){
          setLedsBrightness(getCurrentBrightness()+1);
          Serial.print(" / Up für ");
          Serial.println(identifier);
          if(this->currentBrightness == maxBrightness){
            fadeUp = false;
          }
      }
      if(fadeDown){
          setLedsBrightness(getCurrentBrightness()-1);
          Serial.print(" / Down für ");
          Serial.println(identifier);
          if(this->currentBrightness == 0){
            fadeDown = false;
        }
      }
      if(on){
          setCurrentBrightness(this->maxBrightness);
          setLedsBrightness(this->maxBrightness);
          Serial.println(" ON ");
          Serial.println(identifier);
      }
      if(off){
          setLedsBrightness(0);
          Serial.println(" OFF ");
          Serial.println(identifier);
      }
    }
  }

  void setFadeTime(int upDown, int hour, int minute){
    if (upDown == 1){
      this->upTime = hour * HOUR + minute * MINUTE;
    }
    if (upDown == 0){
      this->downTime = hour * HOUR + minute * MINUTE;
    }
  }
};

LedStrip* ledsTop = new LedStrip(17, "top");
LedStrip* ledsBottom = new LedStrip(16, "bottom");



void setup() {
  
  Serial.begin(9600);

  FastLED.addLeds<NEOPIXEL, 17>(ledsTop->getLedCRGB(),NUM_LEDS);
  FastLED.addLeds<NEOPIXEL, 16>(ledsBottom->getLedCRGB(),NUM_LEDS);

  FastLED.clear();
  FastLED.show();

  ledsTop->setColor(150,255,255);
  ledsBottom->setColor(185,0,255);

  ledsTop->setIntervallMillis();
  ledsBottom->setIntervallMillis();

  WiFi.mode(WIFI_STA);
  WiFi.begin("YOUR SSID", "YOUR PASSWORD");
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

  server.onNotFound([]() {
    server.send(404, "text/plain", "Link wurde nicht gefunden!");
  });

  server.on("/setClock", []() {
    server.send(200, "text/html", printLocalTime());
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    printLocalTime();
  });

  server.on("/on", []() {
    server.send(200, "text/html", "ON");
    ledsTop->autoMode = 0;
    ledsBottom->autoMode = 0;
    FastLED.show();
  });

  server.on("/off", []() {
    server.send(200, "text/html", "OFF");
    //FastLED.setBrightness(0);
    FastLED.show();
  });

  server.on("/setFadeTime", []() {
    //setFadeTime(server.arg(0).toInt(), server.arg(1).toInt(), server.arg(2).toInt());
    server.send(200, "text/html", "Zeit wurde gesetzt!");
  });
  
  server.on("/stripBrightness", []() {
    if (server.arg(0) == "top"){
      ledsTop->setLedsBrightness(server.arg(1).toInt());
    }else if(server.arg(0) == "bottom"){
      ledsBottom->setLedsBrightness(server.arg(1).toInt());
    }
    server.send(200, "text/html", "");
  });

  server.on("/nightMode", []() {
    //nightMode(server.arg(0).toInt());
    server.send(200, "text/html", "NightMode aktiviert!");
  });

  server.on("/setColor", []() {
    //setColor(server.arg(0).toInt(), server.arg(1).toInt(), server.arg(2).toInt(), server.arg(3).toInt());
    server.send(200, "text/html", "Farbe wurde gesetzt!");
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

  server.handleClient();

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
