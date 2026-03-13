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


#define USE_OLED_DISPLAY;
#include <stdint.h>
#include <Wire.h>
#include <SPI.h>
#include <U8g2lib.h>
#include "F4ToserialFont_ded.h"
#include "F4ToserialFont_ffi.h"
#include "F4ToserialFont_pfl.h"


struct Display {
  char* name;
  char* model;
  unsigned char pinsCount;
  unsigned char* pins;
  U8G2* u8g2;
  byte finish;
};

Display* displays = NULL;
unsigned char displaysCount = 0;


void drawLines(Display display, char** lines, unsigned short linesCount) {
  //finishDisplay = false;  
  //display.u8g2->begin();
  display.u8g2->setDrawColor(1);
  display.u8g2->firstPage();
  /* les écrans commencen à  afficher qq chose */
  display.finish = 0;
  do {
    for(unsigned short i = 0; i < linesCount; i++) {
      if(!strcmp(display.name, "FFI")) display.u8g2->drawStr(10, 45,*(lines + i));  else display.u8g2->drawStr(1, (i + 1) * 12, *(lines + i));    
    }
  } while( display.u8g2->nextPage() );
  display.finish = 1;
}








/* ------- OLED DISPLAY ------- */
void add_display(char* name, char* model, unsigned char pinsCount, unsigned char* pins) {
  char* tmp = NULL;
  U8G2* u8g2 = NULL;

  //--- 4 wires (clock, data, cs, dc , reset) ---
  if(!strcmp(model, "SSD1322")) {
    u8g2 = new U8G2_SSD1322_NHD_256X64_2_4W_SW_SPI(U8G2_R2, pins[0], pins[1], pins[2], pins[3], pins[4]);
  }
  else if(!strcmp(model, "SSD1306")) {
    u8g2 = new U8G2_SSD1306_128X64_NONAME_2_4W_SW_SPI(U8G2_R2, pins[0], pins[1], pins[2], pins[3], pins[4]);
  }
  else if(!strcmp(model, "SH1106")) {
    u8g2 = new U8G2_SH1106_128X64_NONAME_2_4W_SW_SPI(U8G2_R2, pins[0], pins[1], pins[2], pins[3], pins[4]);
  }
  else if(!strcmp(model, "SSD1309")) {
    u8g2 = new U8G2_SSD1309_128X64_NONAME2_2_4W_SW_SPI(U8G2_R2, pins[0], pins[1], pins[2], pins[3], pins[4]);
  }
  else if(!strcmp(model, "SSD1325")) {
    u8g2 = new U8G2_SSD1325_NHD_128X64_2_4W_SW_SPI(U8G2_R2, pins[0], pins[1], pins[2], pins[3], pins[4]);
  }
  else if(!strcmp(model, "ST7565")) {
    u8g2 = new U8G2_ST7565_ZOLEN_128X64_2_4W_SW_SPI(U8G2_R2, pins[0], pins[1], pins[2], pins[3], pins[4]);
  }
  else {
    tmp = (char*) malloc((strlen("ERROR: %s: Display model %s not found!") + strlen(name) + strlen(model) + 1) * sizeof(char));
    sprintf(tmp, "ERROR: %s: Display model %s not found!", name, model);
    if(ALLOW_DEBUG) Serial.println(tmp);
    free(tmp);
    return;
  }
  // Add this display to app displays
  displaysCount++;
  displays = (Display*) realloc(displays, displaysCount * sizeof(Display));
  displays[displaysCount-1].name = name;
  displays[displaysCount-1].model = model;
  displays[displaysCount-1].u8g2 = u8g2;
  displays[displaysCount-1].finish = 1;
  
  tmp = (char*) malloc((strlen("Added %s display (model %s)") + strlen(name) + strlen(model) + 1) * sizeof(char));
  sprintf(tmp, "Added %s display (model %s)", name, model);
  if(ALLOW_DEBUG) Serial.println(tmp);
  free(tmp);
}

void clear_displays() {
  for(int i = 0; i < displaysCount; i++) {
    free(displays[i].name);
    free(displays[i].model);
    free(displays[i].u8g2);
    //free(displays[i].finish);
  }
  if(displays != NULL) {
    free(displays);
    displays = NULL;
    displaysCount = 0;
  }
}

void reset_displays() {
  for(int i = 0; i < displaysCount; i++) {
    Display display = displays[i];
    display.u8g2->begin();
    display.u8g2->setDrawColor(1);
    display.u8g2->firstPage();
    if(!strcmp(display.name, "DED"))
      display.u8g2->setFont(u8g2_font_F4ToserialFont_ded);
    else if(!strcmp(display.name, "PFL"))
      display.u8g2->setFont(u8g2_font_F4ToserialFont_pfl);
    else if(!strcmp(display.name, "FFI"))
      display.u8g2->setFont(u8g2_font_F4ToserialFont_ffi);    
    char** array = (char**) malloc(1 * sizeof(char*));
    /*array[0] = (char*) malloc((strlen("I AM ") + strlen(display.name) + 1) * sizeof(char));
    sprintf(array[0], "I AM %s", display.name);*/
    array[0] = (char*) malloc((strlen("") + strlen(display.name) + 1) * sizeof(char));
    sprintf(array[0], "%s", display.name);
    if(ALLOW_DEBUG) Serial.println(array[0]);
    drawLines(display, array, 1);
  }
}




void parse_setup_display(JsonVariant json) {
  if(ALLOW_DEBUG) Serial.println("entering");
  if(!json.is<JsonObject>()) {
      if(ALLOW_DEBUG) Serial.print("Command 'setup_display' MUST be a JSON object. Exiting.");
      return;
  }
  clear_displays();
  if(ALLOW_DEBUG) Serial.println("display cleared");
  JsonObject object = json.as<JsonObject>();
    
  if(ALLOW_DEBUG) Serial.println("Parsing display setup");
  for (JsonObject::iterator kv = object.begin(); kv != object.end(); ++kv) {
    if(ALLOW_DEBUG) Serial.println("Processing display");
    if(!kv->value().is<JsonObject>()) {
      if(ALLOW_DEBUG) Serial.print("Display setup MUST be a JSON object. Exiting.");
      continue;
    }
    JsonObject displayConfiguration = kv->value().as<JsonObject>();

    // Display name
    char* name = strdup(kv->key().c_str());

    // Display model
    if(!displayConfiguration.containsKey("model")) {
      if(ALLOW_DEBUG) Serial.print("Display setup MUST contain a 'model' property. Exiting.");
      continue;
    }
    char* model = strdup(displayConfiguration["model"]);

    // Pins configuration
    if(!displayConfiguration.containsKey("pins")) {
      if(ALLOW_DEBUG) Serial.print("Display setup MUST contain a 'pins' property. Exiting.");
      continue;
    }
    if(!displayConfiguration["pins"].is<JsonArray>()) {
      if(ALLOW_DEBUG) Serial.print("Display setup MUST contain an array 'pins' property. Exiting.");
      continue;
    }
    JsonArray pinsConfiguration = displayConfiguration["pins"].as<JsonArray>();
    unsigned char* pins = (unsigned char*) malloc(pinsConfiguration.size() * sizeof(unsigned char));
    for(int i = 0; i < pinsConfiguration.size(); i++) {
      pins[i] = pinsConfiguration[i].as<unsigned char>();
    }
    add_display(name, model, pinsConfiguration.size(), pins);
    if(ALLOW_DEBUG) Serial.println("Display processed");
  }
  reset_displays();
}



void parse_set_display(JsonVariant json) {
  
  if(!json.is<JsonObject>()) {
    if(ALLOW_DEBUG) Serial.print("Command 'set_display' MUST be a JSON object. Exiting.");
    return;
  }
  JsonObject object = json.as<JsonObject>();
  for (JsonObject::iterator kv = object.begin(); kv != object.end(); ++kv) {
    if(!kv->value().is<JsonArray>()) {
      if(ALLOW_DEBUG) Serial.print("Display setup MUST be a JSON array. Exiting.");
      continue;
    }
    JsonArray lines = kv->value();
    for(int i = 0; i < displaysCount; i++) {
      Display display = displays[i];
      if(!strcmp(display.name, kv->key().c_str())) {
        char** array = (char**) malloc(lines.size() * sizeof(char*));
        for(int j = 0; j < lines.size(); j++) {
          array[j] = (char*) lines[j].as<char*>();
        }
        if(ALLOW_DEBUG) Serial.println(display.finish);
        //  l'Ã©cran n'affiche que si il a terminÃ© son dernier job ! 
         if(display.finish == 1) drawLines(display, array, lines.size());     
    
        free(array);
      }
    }
  }
}
