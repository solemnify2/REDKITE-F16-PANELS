/*

______  ___  _____       _____              _         _
|  ___|/   ||_   _|     /  ___|            (_)       | |
| |_  / /| |  | |  ___  \ `--.   ___  _ __  _   __ _ | |
|  _|/ /_| |  | | / _ \  `--. \ / _ \| '__|| | / _` || |
| |  \___  |  | || (_) |/\__/ /|  __/| |   | || (_| || |
\_|      |_/  \_/ \___/ \____/  \___||_|   |_| \__,_||_|

    Copyright (C) F4ToSerial By Myoda, Inc - All Rights Reserved
    Unauthorized copying of this file, via any medium is strictly prohibitedsss
    Proprietary and confidential
    For more informations, please see : https://f4toserial.com/

*/
#define USE_LIGHTBITS


struct LightBit {
  unsigned char pin = 0;
  bool mode = false;
};

LightBit* lightBits = NULL;
unsigned char LightBitCount = 0;

void clear_lightBit() {
  free(lightBits);
  lightBits = NULL;
  LightBitCount = 0;
}

/*
void add_lightBit(int pin) {
  pinMode(pin, OUTPUT);
}
*/


void add_lightBit(unsigned char pin) {

  if(ALLOW_DEBUG) Serial.println("Add new lightBit");

  lightBits = (LightBit*) realloc(lightBits, (LightBitCount + 1) * sizeof(LightBit));
  lightBits[LightBitCount].pin = pin;
  lightBits[LightBitCount].mode = false;
  pinMode(pin, OUTPUT);

  LightBitCount++;
  if(ALLOW_DEBUG) Serial.println("New lightBit Added");

}


void parse_destroy_lightBits() {
  clear_lightBit();
  if(ALLOW_DEBUG) Serial.println("delete all lightBits");
}




/* ------- LIGHTBITS ------- */
void parse_setup_LightBit(JsonVariant json) {

    if(!json.is<JsonObject>()) {
      return;
    }
    JsonObject object = json.as<JsonObject>();

    if(ALLOW_DEBUG) Serial.println("ready loop SETUP LIGHTBIT");

    clear_lightBit();
    if(ALLOW_DEBUG) Serial.println("LightBit Cleared");

    JsonArray pinsConfiguration = object["pins"].as<JsonArray>();

    if(ALLOW_DEBUG) { Serial.print("Pin number is : ");Serial.print(pinsConfiguration.size());Serial.println(""); }

    int *pins = new int[pinsConfiguration.size()];
    for(size_t i = 0; i < pinsConfiguration.size(); i++) {
      pins[i] = pinsConfiguration[i].as<int>();
      add_lightBit(pins[i]);
    }
}




void parse_set_LightBit(JsonVariant json) {
    if(!json.is<JsonObject>()) {
      return;
    }
    JsonObject object = json.as<JsonObject>();;

    if(ALLOW_DEBUG) Serial.println("----------- ready new Loop -------------");
    JsonArray pinsModes = object["mode"].as<JsonArray>();
    int *modes = new int[pinsModes.size()];


    for(size_t i = 0; i < pinsModes.size(); i++) {
        modes[i] = pinsModes[i].as<int>();
        if(ALLOW_DEBUG) { Serial.print("LightBit ");Serial.print(lightBits[i].pin);Serial.print(" = ");Serial.print(modes[i]); Serial.print(" : "); }
        if(modes[i] == 0) {
         if(lightBits[i].mode == false){
           if(ALLOW_DEBUG) Serial.println("No change !");
         }
         else {
          if(ALLOW_DEBUG) Serial.println("Change to LOW !");
          lightBits[i].mode = false;
          digitalWriteFast(lightBits[i].pin, LOW);
          //digitalWrite(lightBits[i].pin, LOW);
        }
      }
      else {
        if(lightBits[i].mode == true){
          if(ALLOW_DEBUG) Serial.println("No change !");
        }
        else {
          if(ALLOW_DEBUG) Serial.println("Change to HIGH !");
          lightBits[i].mode = true;
          digitalWriteFast(lightBits[i].pin, HIGH);
          //digitalWrite(lightBits[i].pin, HIGH);
        }
      }
    }

    free(modes); //important pour eviter les fuites de mémoire !!!

}
