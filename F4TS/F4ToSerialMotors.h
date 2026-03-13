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

#define USE_MOTORS;
#include <AccelStepper.h>
#include <MultiStepper.h>
#define DEFAULT_ACCELERATION  1000
#define DEFAULT_MAXSPEED      1000
#define PINS_COUNT            4



/* ------- MOTOR ------- */

struct Motor {
    char* name;
    unsigned int acceleration;
    unsigned int maxSpeed;
    short steps;
    unsigned char* pins;
    AccelStepper* stepper;
};
Motor* motors = NULL;
unsigned char motorsCount = 0;



void reset_motors() {
    if(ALLOW_DEBUG) { Serial.println("calibrating Motors positions");}
    MultiStepper* multiStepper = new MultiStepper();
    long int* downPositions = NULL;
    long int* upPositions = NULL;
    for(int i = 0; i < motorsCount; i++) {
        downPositions = (long int*) realloc(downPositions, (i+1) * sizeof(long int));
        downPositions[i] = -50;
        upPositions = (long int*) realloc(upPositions, (i+1) * sizeof(long int));
        upPositions[i] = motors[i].steps + 50;
        motors[i].stepper->setMaxSpeed(200);
        motors[i].stepper->setAcceleration(200);
        multiStepper->addStepper(*motors[i].stepper);
    }

    multiStepper->moveTo(upPositions);
    multiStepper->runSpeedToPosition();
    if(ALLOW_DEBUG) { Serial.println("Calibrating to max Position"); }
    delay(300);
    multiStepper->moveTo(downPositions);
    multiStepper->runSpeedToPosition();
    if(ALLOW_DEBUG) { Serial.println("Calibrating to min Position"); }
    for(int i = 0; i < motorsCount; i++) {
        motors[i].stepper->setCurrentPosition(0);
        motors[i].stepper->setMaxSpeed(motors[i].maxSpeed);
        motors[i].stepper->setAcceleration(motors[i].acceleration);
    }
    free(downPositions);
    free(upPositions);
    free(multiStepper);

}

void add_motor(char* name, unsigned short steps, unsigned int acceleration, unsigned int maxSpeed, unsigned char* pins) {

    if(ALLOW_DEBUG) { Serial.print("Add a new Motor "); Serial.print(motorsCount); Serial.print(" "); Serial.println(name); }
  
    AccelStepper* stepper = new AccelStepper(PINS_COUNT, pins[0], pins[1], pins[2], pins[3]);
    motors = (Motor*) realloc(motors, (motorsCount + 1) * sizeof(Motor));
    motors[motorsCount].name = name;
    motors[motorsCount].steps = steps;
    motors[motorsCount].acceleration = acceleration;
    motors[motorsCount].maxSpeed = maxSpeed;
    motors[motorsCount].pins = pins;
    motors[motorsCount].stepper = stepper;
    motorsCount++;
    if(ALLOW_DEBUG) { Serial.print("Free Memory = ");Serial.println(freeMemory()); }
}

void clear_motors() {
  if(ALLOW_DEBUG) { Serial.println("clearing Motors");}
    for(int i = 0; i < motorsCount; i++) {
        free(motors[i].name);
        free(motors[i].pins);
        free( motors[i].stepper);
    }
    free(motors);
    motors = NULL;
    motorsCount = 0;
}



void parse_setup_command(JsonVariant json) {
    
    if(ALLOW_DEBUG) { Serial.println("Processing setup motors");}
  
    if(!json.is<JsonObject>()) {
        return;
    }
    clear_motors();

    
    
    // v5 JsonObject object = json;    
    JsonObject object = json.as<JsonObject>();
    for (JsonObject::iterator kv=object.begin(); kv!=object.end(); ++kv) {

        if(ALLOW_DEBUG) { Serial.print("Receive info for motor : ");Serial.println(kv->key().c_str()); }
        
        
        if(!kv->value().is<JsonObject>()) {
            continue;
        }
        
        //v5 JsonObject motorConfiguration = kv->value();
        JsonObject motorConfiguration = kv->value().as<JsonObject>();

        char* name = (char*) malloc((strlen(kv->key().c_str()) + 1) * sizeof(char));
        
        memcpy(name, kv->key().c_str(), (strlen(kv->key().c_str()) + 1) * sizeof(char));
        // Steps
        if(!motorConfiguration.containsKey("steps")) {
            continue;
        }
        if(!motorConfiguration["steps"].is<short>()) {
            continue;
        }
        short steps = motorConfiguration["steps"].as<short>();
        // Acceleration
        unsigned int acceleration = DEFAULT_ACCELERATION;
        if(motorConfiguration.containsKey("acceleration"))
            acceleration = motorConfiguration["acceleration"].as<unsigned int>();
        // Max speed
        unsigned int maxSpeed = DEFAULT_MAXSPEED;
        if(motorConfiguration.containsKey("maxSpeed"))
            maxSpeed = motorConfiguration["maxSpeed"].as<unsigned int>();
        // Pins configuration
        if(!motorConfiguration.containsKey("pins")) {
            continue;
        }
        if(!motorConfiguration["pins"].is<JsonArray>()) {
            continue;
        }
        JsonArray pinsConfiguration = motorConfiguration["pins"].as<JsonArray>();
        unsigned char* pins = (unsigned char*) malloc(PINS_COUNT * sizeof(unsigned char));

        
        if(pinsConfiguration.size() != PINS_COUNT) {
            continue;
        }
        
        for(int i = 0; i < PINS_COUNT; i++) {
            pins[i] = pinsConfiguration[i].as<unsigned char>();
            if(ALLOW_DEBUG) { Serial.print("Add pin : "); + Serial.println(pins[i]);}
        }
        add_motor(name, steps, acceleration, maxSpeed, pins);
    }
    reset_motors();
}



void parse_setstep_command(JsonVariant json) {

    if(ALLOW_DEBUG) { Serial.println("Processing new step for motors");}
  
    if(!motorsCount) {
        return;
    }
    JsonObject object = json.as<JsonObject>();
    
     for (JsonObject::iterator kv=object.begin(); kv!=object.end(); ++kv) {
          if(!kv->value().is<short>()) {
              continue;
          }
          bool stepsSet = false;
          short steps = kv->value().as<short>();

          for(int i = 0; i < motorsCount; i++) {
              Motor motor = motors[i];
              if(!strcmp(motor.name, kv->key().c_str())) {
                  motor.stepper->moveTo(steps);

                  
                  if(ALLOW_DEBUG) { Serial.print("Motor move to "); Serial.println(steps);}
                  
                  stepsSet = true;
              }
          }
          if(!stepsSet) {
              continue;
          }
        }
}
