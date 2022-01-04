#ifndef LedStrip_h
#define LedStrip_h

//set number of used LEDs per strip in NUM_LEDS 
#define HOUR 3600
#define MINUTE 60
#define NUM_LEDS 40

class LedStrip : CFastLED {
  private:
  bool fadeUp = false;
  bool fadeDown = false;
  bool autoMode = true;
  bool on = false;
  bool off = false;
  int upTime;
  int upTimeHour;
  int upTimeMinute;
  int downTime;
  int downTimeHour;
  int downTimeMinute;
  int currentBrightness;
  int maxBrightness;
  int nightModeDuration = 0;
  int h = 255;
  int s = 255;
  int v = 255;
  int intervallDuration = 2;
  long intervallMillis;
  int pin;
  String identifier;
  CRGB leds[NUM_LEDS]; 
  public:

  //Constructor
  LedStrip(uint8_t pin, String identifier){
    this->pin = pin;
    this->identifier = identifier;
  }

  //----------Setter----------//

  /**
  * Creates a timestamp with time in ms for time based brightness cycle check
  */
  void setIntervallMillis(){
    this->intervallMillis = millis();
  }

  /**
  * Set AutoMode to 0 (fadeTime is not observed) or to 1 (fading at fadeTime)
  */
  void setAutoMode(int state){
    if(state == 0){
      autoMode = 0;
    }else if(state == 1){
      autoMode = 1;
    }
  }

  /**
  * Set maxBrightness to limit current or lower brightness between 1 and 255
  */
  void setMaxBrightness(int maxBrightness){
    if(maxBrightness <= 255 && maxBrightness > 0){
      this->maxBrightness = maxBrightness;
    }
  }

  /**
  * Set currentBrightness, which holds the leds brightness between 0 and 255
  */
  int setCurrentBrightness(int brightness){
    if(brightness <=255 && brightness >= 0){
      this->currentBrightness = brightness;
    }
  }

  /**
  * Set brightness for the complete ledStrip 
  */
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

  /**
  * Set fading up and fading down time for json and calculate internal fading times.
  */
  void setFadeTime(String upDown, int hour, int minute){
    if (upDown == "up"){
      this->upTimeHour = hour;
      this->upTimeMinute = minute;
      this->upTime = hour * HOUR + minute * MINUTE;
    }
    if (upDown == "down"){
      this->downTimeHour = hour;
      this->downTimeMinute = minute;
      this->downTime = hour * HOUR + minute * MINUTE;
    }
  }
  /**
  * Set H (hue) S (saturation) V (value) color between 0 and 255
  */
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

  //----------Getter----------//

  int getCurrentBrightness(){
    return this->currentBrightness;
  }

  int getMaxBrightness(){
    return this->maxBrightness;
  }

  CRGB* getLedCRGB(){
    return this->leds;
  }

  //----------Logic----------//

  /**
  * Activate nightMode, which deactives every second led in a strip and set the other leds V/brightness to 100
  */
  void nightMode(int dauer) {
    for (int i = 0; i < NUM_LEDS; i++) {
      if ((i % 2) == 0) {
        CHSV color = CHSV(h, s, (int) maxBrightness/3);
        fill_solid(leds, NUM_LEDS, color);
      } else {
        CHSV color = CHSV(h, s, 0);
        fill_solid(leds, NUM_LEDS, color);
      }
    }
    nightModeDuration = dauer * 60000 + intervallMillis;
  }

  /**
  * Toggle bools to indicate which mode (stable OR fading) is/will be active
  */
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

      if(nightModeDuration = 0){
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
      }else{
        if(nightModeDuration <= intervallMillis){
          nightModeDuration = 0;
        }
      }
    }
  }

  String exportAsJson(){
    StaticJsonDocument<300> doc;
    doc["identifier"] = this->identifier;
    doc["autoMode"] = this->autoMode;
    doc["upTimeHour"] = this->upTimeHour;
    doc["upTimeMinute"] = this->upTimeMinute;
    doc["downTimeHour"] = this->downTimeHour;
    doc["downTimeMinute"] = this->downTimeMinute;
    doc["upTime"] = this->upTime;
    doc["downTime"] = this->downTime;
    doc["currentBrightness"] = this->currentBrightness;
    doc["maxBrightness"] = this->maxBrightness;
    doc["nightModeDuration"] = this->nightModeDuration;
    doc["h"] = this->h;
    doc["s"] = this->s;
    doc["v"] = this->v;
       
    String json;
    serializeJson(doc, json);
    return json;
  }
};
#endif