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
    Written by Alexandre Laurencin <laurencin.alexandre@gmail.com>, 14/03/2020
    
    For more informations, please see : https://f4toserial.com/

*/

/* 
 *  FR : Arduino supporte les vitesses de transfert suivante : 300, 600, 1200, 2400, 4800, 9600, 14400, 19200, 28800, 38400, 57600, or 115200 bps.
 *  Par défault le programme est configuré en 115200 bps. Si vous ne voulez pas avoir de décrochage pour l'affichage du DED, augmentez la vitesse à 1.000.000 bps. 
 *    
 *  EN : Arduino support up to 300, 600, 1200, 2400, 4800, 9600, 14400, 19200, 28800, 38400, 57600, or 115200 bps. 
 *  By default this program is configured at 115200 bps. If you get stall on your DED, please incrase baud rate to 1.000.000 bps.  
 *    
*/
  
#define BAUDRATE    115200 /* in bps */
#define ALLOW_DEBUG  false
#define BUFFERSIZE  1024




/* ----------- Ne plus toucher en dessous de cette ligne | Do not touch under this line ------------ */


#include "MemoryFree.h"
#include <ArduinoJson.h>
#include "digitalWriteFast.h"
#include "F4ToSerialMotors.h"
#include "F4ToSerialLightBits.h"
#include "F4ToSerialMatrixLightBits.h"
#include "F4ToSerialBCD.h"
#include "F4ToSerialCommons.h"
#include "F4ToSerialOledDisplays.h"

int firstLoop = false;


void setup() {
    Serial.begin(BAUDRATE);
    if(ALLOW_DEBUG){
      Serial.println("Let's Go Baby ! ");
    }


    /* Internal LED Blinking 5 times to indicate that the board is ready ! */
    for(int i=0; i<=10; i++)
    {
      pinMode(13, OUTPUT); 
      if(i%2 ==0)digitalWrite(13,LOW);
      else digitalWrite(13,HIGH);
      delay(200);      
    }
}

void loop() {
    static char buffer[BUFFERSIZE];
    if (readline(Serial.read(), buffer, BUFFERSIZE) > 0) {


        DynamicJsonDocument doc(BUFFERSIZE);
        DeserializationError error = deserializeJson(doc, buffer);
        if (error) {
            if(ALLOW_DEBUG) {
              Serial.println("Deserialization Error ");
              Serial.println(error.c_str());           
            }
            return;
        }
        
        #ifdef USE_MOTORS
        if(doc.containsKey("setup_stepper")) {
            parse_setup_command(doc["setup_stepper"]);
        }
        
        if(doc.containsKey("setstep")) {
            parse_setstep_command(doc["setstep"]);
        }
        #endif

        #ifdef USE_BCD
        if(doc.containsKey("setup_bcd") && doc.containsKey("digitOn") && doc.containsKey("digitOff")) {
            parse_setup_bcd(doc);           
        }
        if(doc.containsKey("set_seg")) {
            parse_set_seg_command(doc["set_seg"]);
        }
        if(doc.containsKey("destroy_all_bcd")) {
            parse_destroy_bcd();
        }
        #endif
        
        #ifdef USE_OLED_DISPLAY
        if(doc.containsKey("setup_display")) {
            parse_setup_display(doc["setup_display"]);
        }
        else if(doc.containsKey("set_display")){
          parse_set_display(doc["set_display"]);
        }
        #endif

        #ifdef USE_LIGHTBITS
        if(doc.containsKey("setup_LightBit")) {
            parse_destroy_lightBits();
            parse_setup_LightBit(doc["setup_LightBit"]);
        }
        else if(doc.containsKey("set_LightBit")){
          parse_set_LightBit(doc["set_LightBit"]);
        } 
        #endif

        #ifdef USE_MATRIX
        if(doc.containsKey("setup_matrix")) {
          parse_setup_matrice(doc["setup_matrix"]);
        }
        else if(doc.containsKey("destroy_matrix")) {
          parse_destroy_matrix();
        }
        else if(doc.containsKey("update_matrix")){
          parse_set_matrice(doc["update_matrix"]);
        }
        #endif 
        
  
        if(ALLOW_DEBUG) { Serial.print("Free Memory = ");Serial.println(freeMemory()); }
        
        return;
    }


    #ifdef USE_MATRIX
    startRefreshMatrix();
    #endif

    #ifdef USE_MOTORS
    for(int i = 0; i < motorsCount; i++) {
        motors[i].stepper->run();
    }
    #endif
    
    #ifdef USE_BCD
    startRefreshBCD();
    #endif USE_BCD
    

    if(firstLoop == false) { if(ALLOW_DEBUG) { Serial.print("Free Memory = ");Serial.println(freeMemory()); } }
    firstLoop = true;
}
