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

#define USE_BCD;

/*

En mode dÃ©but, modifier ces valeurs par : 

long digitOff = 200;
long digitOn = 1000000;

en prod utiliser : 


long digitOff = 200;
const long digitOn = 3000;

*/
long digitOff = 200;
long digitOn = 3000;

  /* Dns le cas d'un afficher 7 segments common Cathode, il faut inversser HIGH par LOW et inverssement sur tous les digitalWrite(bcd.dot... . */




//-------------- dÃ©but des ennuis ---- ;-)

/* ------- bcd ------- */

struct BCD {
    char* name;
    unsigned char pinsCount; /* nombre de pin (non utilisÃ©) */
    unsigned char* bcd_pins; /* broche ABCD du dÃ©coder */
    unsigned char* seg_pins; /* broche des digits */ 
    long digitalValue; /* broche ABCD du dÃ©coder */
    short int digitNumber; /* digital Value nombre de Digit */
    short int dot;
    short int loopDigit; /* dernier digit affichÃ© */
};
BCD* bcds = NULL;
unsigned char bcdsCount = 0;




int lastBcd = 0;
unsigned long previousMillis = 0;
bool onchangeOn = false;
bool onchangeOff = false;






void add_bcd(char* name, unsigned char* bcd_pins, unsigned char*seg_pins, byte segmdigitalValue, byte digitNumber, short int dot) {
    char* tmp = NULL;
    bcds = (BCD*) realloc(bcds, (bcdsCount + 1) * sizeof(BCD));
    bcds[bcdsCount].name = name;
    bcds[bcdsCount].bcd_pins = bcd_pins;
    bcds[bcdsCount].seg_pins = seg_pins;
    bcds[bcdsCount].digitalValue = -1;
    bcds[bcdsCount].digitNumber = digitNumber;
    bcds[bcdsCount].loopDigit = 0;
    bcds[bcdsCount].dot = dot;


    
    if(ALLOW_DEBUG) { Serial.print("Size Of Segment Pin = "); Serial.print(digitNumber); Serial.println("");}

    
    //initialisation des pins en OUTPUT et mise Ã  l'Ã©tat LOW
    for(int i=0; i<4; i++) { 
      if(ALLOW_DEBUG) { 
        Serial.print("bcd_pins=");
        Serial.print(bcd_pins[i]);
        Serial.println(""); 
      }
      pinMode(bcds[bcdsCount].bcd_pins[i],OUTPUT);
      digitalWrite(bcds[bcdsCount].bcd_pins[i],LOW); 
    }

    //initialisation des pins en OUTPUT et mise Ã  l'Ã©tat LOW
    for(int i=0; i<segmdigitalValue; i++) { 
      if(ALLOW_DEBUG) { 
        Serial.print("seg_pins=");
        Serial.print(seg_pins[i]);
        Serial.println(""); 
      }
      pinMode(bcds[bcdsCount].seg_pins[i],OUTPUT);
      digitalWrite(bcds[bcdsCount].seg_pins[i],LOW); 
    }

    if(bcds[bcdsCount].dot > 0) { pinMode(bcds[bcdsCount].dot,OUTPUT); }
    
    if(ALLOW_DEBUG) { 
      Serial.print("Dot Pin : ");
      if(bcds[bcdsCount].dot > 0) { Serial.println(bcds[bcdsCount].dot); } else  { Serial.println("No dot !"); }
    }


    tmp = (char*) malloc((strlen("Added %s bcd") + strlen(name) + 1) * sizeof(char));
    sprintf(tmp, "Added %s bcd", name);
    if(ALLOW_DEBUG) Serial.println(tmp);
    free(tmp);

    bcdsCount++;

}

// digitalValue ^ index: power function for integers
long power (long digitalValue, int index) {
    if (index == 0) {
        return 1;
    }
    long num = digitalValue;
    for (int i = 1; i < index; i++) {
        digitalValue = digitalValue * num;
    }
    return digitalValue;
}





void splitIntegerIntoArray(int arr[], int size, long digitalValue) {
    long tempdigitalValue = digitalValue;
    int digit = 0;
    for (int i = 1; i <= size; i++) {
        int index = size - i;
        digit = tempdigitalValue / power(10, index);       
        arr[i-1] = digit;
        tempdigitalValue = tempdigitalValue - power(10, index) * digit;
    }
}


void clear_bcds() {
    for(int i = 0; i < bcdsCount; i++) {
        free(bcds[i].name);
        /*free(bcds[i].pinsCount);*/
        free(bcds[i].bcd_pins);
        free(bcds[i].seg_pins);
        bcds[i].digitalValue = -1;
        bcds[i].digitNumber = 0;
        bcds[i].loopDigit = 0;
    }
    free(bcds);
    bcds = NULL;
    bcdsCount = 0;
    lastBcd = 0;
}




void parse_destroy_bcd() {
  if(ALLOW_DEBUG) Serial.println("Destory all BCD!");
  for(int i = 0; i < bcdsCount; i++) {
      BCD bcd = bcds[i];
      for(int j=0;j<bcd.digitNumber;j++) digitalWrite(bcd.seg_pins[j],LOW); 
      for(int j=0;j<bcd.digitNumber;j++) digitalWrite(bcd.bcd_pins[j],LOW); 
    }
  
  clear_bcds();
}


void parse_setup_bcd(DynamicJsonDocument json) {
  
  if(ALLOW_DEBUG) Serial.println("entering");
  if(!json["setup_bcd"].is<JsonObject>()) {
      if(ALLOW_DEBUG) Serial.print("Command 'setup_bcd' MUST be a JSON object. Exiting.");
      return;
  }
  
  if(ALLOW_DEBUG) Serial.println("bcd cleared");
  parse_destroy_bcd(); 
  
  long ConfigDigitOff = json["digitOff"];
  long ConfigDigitOn = json["digitOn"];
  if(ALLOW_DEBUG) { Serial.print("digitOn = "); Serial.println(ConfigDigitOn); }
  if(ALLOW_DEBUG) { Serial.print("digitOff = "); Serial.println(ConfigDigitOff); }
  
  
  JsonObject object = json["setup_bcd"];
  digitOff = ConfigDigitOff;
  digitOn = ConfigDigitOn;
  

    
  if(ALLOW_DEBUG) Serial.println("Parsing bcd setup");
  for (JsonObject::iterator kv = object.begin(); kv != object.end(); ++kv) {
    if(ALLOW_DEBUG) Serial.println("Processing bcd");
    if(!kv->value().is<JsonObject>()) {
      if(ALLOW_DEBUG) Serial.print("bcd setup MUST be a JSON object. Exiting.");
      continue;
    }
    JsonObject bcdConfiguration = kv->value();

    // Pins configuration
    if(!bcdConfiguration.containsKey("bcd_pins")) {
      if(ALLOW_DEBUG) Serial.print("bcd setup MUST contain a 'bcd_pins' property. Exiting.");
      continue;
    }
    
    if(!bcdConfiguration["seg_pins"].is<JsonArray>()) {
      if(ALLOW_DEBUG) Serial.print("bcd setup MUST contain an array 'seg_pins' property. Exiting.");
      continue;
    }

    char* name = strdup(bcdConfiguration["name"]);
    
    JsonArray bcd_pinsConfiguration = bcdConfiguration["bcd_pins"].as<JsonArray>();
    unsigned char* bcd_pins = (unsigned char*) malloc(bcd_pinsConfiguration.size() * sizeof(unsigned char));
    for(int i = 0; i < bcd_pinsConfiguration.size(); i++) {
      bcd_pins[i] = bcd_pinsConfiguration[i].as<unsigned char>();
    }

    JsonArray seg_pinsConfiguration = bcdConfiguration["seg_pins"].as<JsonArray>();
    unsigned char* seg_pins = (unsigned char*) malloc(seg_pinsConfiguration.size() * sizeof(unsigned char));
    for(int i = 0; i < seg_pinsConfiguration.size(); i++) {
      seg_pins[i] = seg_pinsConfiguration[i].as<unsigned char>();
    }

    short int dotPin = 0;
    
    if(bcdConfiguration.containsKey("dot")) {
      dotPin = bcdConfiguration["dot"].as<short int>();
      if(ALLOW_DEBUG) Serial.print("Dot found = ");
      if(ALLOW_DEBUG) Serial.println(dotPin);
    }

    add_bcd(name, bcd_pins, seg_pins, seg_pinsConfiguration.size(), bcdConfiguration["seg_pins"].size(), dotPin );
    if(ALLOW_DEBUG) Serial.println("bcd processed");
  }
}




void updateSegmentValue(char* name, long segment_value)
{
  if(ALLOW_DEBUG) Serial.println("change value of segment");

  for(int i = 0; i < bcdsCount; i++) {
      BCD bcd = bcds[i];
      if(ALLOW_DEBUG) { 
        Serial.print("Modification du BCD : ");
        Serial.println(bcd.name);
      }      
      if(!strcmp(bcd.name, name)) {
        if(ALLOW_DEBUG) Serial.println("found name ");
        bcds[i].digitalValue = segment_value;
        bcds[i].loopDigit = 0;        
      }
    }
}






void parse_set_seg_command(JsonVariant json) {
      if(!json.is<JsonObject>()) {
      return;
    }
    JsonObject segmentConfiguration = json.as<JsonObject>();;

    char* name = strdup(segmentConfiguration["name"]);   
    int arraySize = segmentConfiguration["value"].size();   //get size of JSON Array
    if(ALLOW_DEBUG) Serial.print("\nSize of value array: ");
    if(ALLOW_DEBUG) Serial.println(arraySize);

    /*correction d'un bug !!!! */
    if(arraySize<1) arraySize=1;
  
 
    int loopDigitValue[arraySize] = {0}; 

    
    if(ALLOW_DEBUG) Serial.println("\nArray values with explicit casting");
    for (int i = 0; i < arraySize; i++) {  //Iterate through results
      if(ALLOW_DEBUG) Serial.println(segmentConfiguration["value"][i].as<int>());//Explicit cast
      loopDigitValue[i] = segmentConfiguration["value"][i].as<int>();
    }

    /* transformation int en long */
    long value = 0;
    for (int i=0; i<arraySize; i++)
    {
      value *= 10;
      value += loopDigitValue[i];
    }

    if(ALLOW_DEBUG) Serial.print("Long value received = ");
    if(ALLOW_DEBUG) Serial.println(value);
    updateSegmentValue(name, value);
    
    free(name);
    
    return;
    
 
}










void CycleDigitOff()
{
  if(onchangeOff == true) return;
  onchangeOff = true;
  onchangeOn = false;
  //--------------------
  //if(ALLOW_DEBUG) { Serial.println("------------- CycleDigitOff ------------- "); }
  BCD bcd = bcds[lastBcd]; //rÃ©cupÃ©ration du bcd
  if(bcd.digitalValue <0) return;

  for(int ndigit = 0; ndigit < bcd.digitNumber;ndigit++) { 
      digitalWrite(bcd.seg_pins[ndigit], LOW);
      //if(ALLOW_DEBUG) { Serial.print("extinction du pin : ");  Serial.print(bcd.seg_pins[ndigit]); Serial.println("");} 
  }


  if(bcd.dot >0) 
  {
    //if(ALLOW_DEBUG) Serial.println("bcd.dot is more > 0");
    digitalWrite(bcd.dot, HIGH);
    if(bcd.loopDigit == 1)
    {
      digitalWrite(bcd.dot, LOW);
    }
  } else {
    //if(ALLOW_DEBUG) Serial.println("bcd.dot is 0, no dot ! ");
  }


  if(bcds[lastBcd].loopDigit < bcd.digitNumber-1) {
    bcds[lastBcd].loopDigit++;
  } else {
    bcds[lastBcd].loopDigit = 0;
    lastBcd++;
  }
  if(lastBcd >= bcdsCount) lastBcd = 0; // si on a pas plus de bcd dÃ©clarÃ© on reset
  
}


void CycleDigitOn()
{ 
  if(onchangeOn == true) return;
  onchangeOn = true;
  onchangeOff = false;
  //--------------------
  //if(ALLOW_DEBUG) { Serial.println("------------- CycleDigitOn ------------- "); }  
  BCD bcd = bcds[lastBcd]; //rÃ©cupÃ©ration du bcd

  if(bcd.digitalValue <0) return;
  
  

  int arr[bcd.digitNumber]; //tableau des digits Ã  afficher
  memset(arr, 0, sizeof(int));
  splitIntegerIntoArray(arr, bcd.digitNumber, bcd.digitalValue); // on place chaque digit dans un tableau

  /*if(ALLOW_DEBUG) { 
    Serial.print("BCD en cours = ");  Serial.print(bcd.name); Serial.println("");
    Serial.print("Nombre Ã  afficher = "); Serial.print(bcd.digitalValue); Serial.println("");
    Serial.print("chiffre Ã  afficher = "); Serial.print(arr[bcd.loopDigit]); Serial.println("");
    Serial.print("Nombre de digit = "); Serial.print(bcd.digitNumber); Serial.println("");
    Serial.print("Dernier Digit affichÃ© = "); Serial.print(bcd.loopDigit); Serial.println("");
  }*/

  //digitalWrite(bcd.seg_pins[bcd.loopDigit], HIGH);    
  /*if(ALLOW_DEBUG) { 
    Serial.print("BCD["); Serial.print(bcd.name);  Serial.print( "]  A = "); Serial.print(bitRead(arr[bcd.loopDigit], 0)); Serial.println("");
    Serial.print("BCD["); Serial.print(bcd.name);  Serial.print( "]  B = "); Serial.print(bitRead(arr[bcd.loopDigit], 1)); Serial.println("");
    Serial.print("BCD["); Serial.print(bcd.name);  Serial.print( "]  C = "); Serial.print(bitRead(arr[bcd.loopDigit], 2)); Serial.println("");
    Serial.print("BCD["); Serial.print(bcd.name);  Serial.print( "]  D = "); Serial.print(bitRead(arr[bcd.loopDigit], 3)); Serial.println("");   
  }*/
  digitalWrite(bcd.bcd_pins[0], bitRead(arr[bcd.loopDigit], 0));
  digitalWrite(bcd.bcd_pins[1], bitRead(arr[bcd.loopDigit], 1));
  digitalWrite(bcd.bcd_pins[2], bitRead(arr[bcd.loopDigit], 2));
  digitalWrite(bcd.bcd_pins[3], bitRead(arr[bcd.loopDigit], 3));


  
  digitalWrite(bcd.seg_pins[bcd.loopDigit], HIGH);
  //if(ALLOW_DEBUG) { Serial.print("alumage du pin : ");  Serial.print(bcd.seg_pins[bcd.loopDigit]); Serial.println("");} 

 
  
    
}


void startRefreshBCD() {
  if(bcdsCount<=0) return; // aucun BCD dÃ©clarÃ©
  BCD bcd = bcds[lastBcd]; //rÃ©cupÃ©ration du bcd
  if(bcd.digitalValue<0) //a l'initialisation on lui donne -1. donc bcd sans valeur digitale.
  {
    lastBcd++;
    if(lastBcd >= bcdsCount) lastBcd = 0; // si on a pas de bcd dÃ©clarÃ© on reset
  }

  unsigned long currentMillis = micros();
  if(currentMillis - previousMillis >= digitOff) {  
    if((currentMillis - previousMillis) <= digitOn) {
      CycleDigitOn();
    }
    else previousMillis = currentMillis;
  }
  else {
    CycleDigitOff(); 
  }
}
