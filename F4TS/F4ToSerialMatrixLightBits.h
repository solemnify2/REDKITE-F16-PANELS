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


#define USE_MATRIX;

/* ------- MATRIX LIGHTBIT ------- */


struct Matrice {
	char* name;
	unsigned char rows;
	unsigned char cols;
	unsigned char pinStart;
	unsigned char** datatable = NULL;
	unsigned char* pinsRows = NULL;
	unsigned char* pinsCols = NULL;
};
Matrice* matrices = NULL;
unsigned char matricesCount = 0;
unsigned long MicroRefresh=500;
unsigned long st=0;
byte loopMatrix = 0;
static byte r=0;
static byte c=0;



void clear_matrice() {
	for(int i = 0; i < matricesCount; i++) {
		free(matrices[i].name);

		for(int r =0; r<matrices[i].rows; r++) {
			free(matrices[i].datatable[r]);
		}
		free(matrices[i].datatable);
		free(matrices[i].pinsRows);
		free(matrices[i].pinsCols);
	}
	free(matrices);
	matrices = NULL;
	matricesCount = 0;
}



void parse_destroy_matrix() {
	clear_matrice();
	if(ALLOW_DEBUG) Serial.println("delete all matrix");
}


void add_matrice(char* name, unsigned char rows, unsigned char cols, unsigned char pinStart) {
	if(ALLOW_DEBUG) Serial.println("Add new matrix");

	matrices = (Matrice*) realloc(matrices, (matricesCount + 1) * sizeof(Matrice));
	matrices[matricesCount].name = name;
	matrices[matricesCount].rows = rows;
	matrices[matricesCount].cols = cols;
	matrices[matricesCount].pinStart = pinStart;
//construction de la matrice
	matrices[matricesCount].datatable = (unsigned char**) malloc(rows*sizeof(unsigned char*));
	for(int r = 0; r < rows; r++) {
		matrices[matricesCount].datatable[r] = (unsigned char*) malloc(cols*sizeof(unsigned char));
		for(int c = 0; c < cols; c++) {
			matrices[matricesCount].datatable[r][c] = 0;
		}
	} 
//construction des lignes    
	matrices[matricesCount].pinsRows = (unsigned char*)malloc(rows*sizeof(char));
	for(int i = 0; i < rows; i++) {
		matrices[matricesCount].pinsRows[i] = pinStart + cols + i;
	}
//construction des colonnes
	matrices[matricesCount].pinsCols = (unsigned char*)malloc(cols*sizeof(char));
	for(int i =0 ; i< cols; i++) { 
		matrices[matricesCount].pinsCols[i] = i + pinStart;
	}

  //initialisation des pins en OUTPUT et mise Ã  l'Ã©tat LOW
	for(int i=0; i<cols; i++) { 
		pinMode(matrices[matricesCount].pinsCols[i],OUTPUT);
		digitalWrite(matrices[matricesCount].pinsCols[i],LOW); 
	}
  //on passe les pins Ã  l'Ã©tat HIGH
	for(int i =0; i<rows; i++) {
		pinMode(matrices[matricesCount].pinsRows[i],OUTPUT);
		digitalWrite(matrices[matricesCount].pinsRows[i],HIGH);
  //if(ALLOW_DEBUG) Serial.println(i);
	}

	matricesCount++;
	if(ALLOW_DEBUG) Serial.println("New matrix Added");

}





void parse_set_matrice(JsonVariant json) {
	if(ALLOW_DEBUG) Serial.println("update matrix");
	if(!matricesCount) {
		if(ALLOW_DEBUG) Serial.println("please setup matrice before !");
		return;
	}

	if(ALLOW_DEBUG) Serial.println("entering reading matrice on serial port ...");
	if(!json.is<JsonObject>()) {
//if(ALLOW_DEBUG) Serial.print"Command 'setup_display' MUST be a JSON object. Exiting.");
		return;
	}

	JsonObject object = json.as<JsonObject>();

//if(ALLOW_DEBUG) Serial.println("Parsing matrice set");

	for (JsonObject::iterator kv = object.begin(); kv != object.end(); ++kv) {
//if(ALLOW_DEBUG) Serial.println("Processing matrice send data");
		if(!kv->value().is<JsonObject>()) {
			if(ALLOW_DEBUG) Serial.print ("matrice setup MUST be a JSON object. Exiting.");
			continue;
		}


		JsonObject matriceConfiguration = kv->value();
		JsonArray matriceJson = matriceConfiguration["matrice"].as<JsonArray>();

		for(int i = 0; i < matricesCount; i++) {
			Matrice matrice = matrices[i];
			if(!strcmp( matrice.name, kv->key().c_str())) {
				if(ALLOW_DEBUG) Serial.println("strcmp ! Continue ! ");
				for(int r =0; r<matrice.rows; r++) {
					if(ALLOW_DEBUG) Serial.print("[");
					JsonArray line = matriceJson[r].as<JsonArray>();
					if(matriceJson.size() != matrice.rows) { 
						break; 
					} 
					for(int c=0; c<matrice.cols; c++) { 
						if(line.size() != matrice.cols) { 
							break; 
						} 
						char tmp = line[c].as<unsigned char>();
						switch(tmp)
						{
							case 0:
							  matrice.datatable[r][c] = 0;
							  if(ALLOW_DEBUG) Serial.print("0");
							break;   
							case 1:
							  matrice.datatable[r][c] = 1;
							  if(ALLOW_DEBUG) Serial.print("1");
							break;   
							default:
							  if(ALLOW_DEBUG) Serial.print("0");
							break;
						}
					}
					if(ALLOW_DEBUG) Serial.println("]");
				}
				if(ALLOW_DEBUG) Serial.println("New Data Received !");
			}
		}
	}

	
}



// fonction d'affichage
void startRefreshMatrix(){

  if(loopMatrix<matricesCount){
    Matrice matrice = matrices[loopMatrix]; 

    if(r<matrice.rows){
      //on parcourt les lignes
      if(c<matrice.cols){
        //puis chaque case de la ligne (colonne)
        if(matrice.datatable[r][c]==1) {
          if(!st)st=micros();// demarrage du compteur si c'est pas le cas

          //on allume ou non en fonction du tableau matrice
          digitalWrite(matrice.pinsRows[r],LOW);
          digitalWrite(matrice.pinsCols[c],HIGH);

          if((micros()-st)>MicroRefresh){
          //la durée est passae on change de led et on reinitialise le compteur
          //on éteint et on place Ã  l'Ã©tat HIGH pour la suivante
            digitalWrite(matrice.pinsRows[r],HIGH);
            digitalWrite(matrice.pinsCols[c],LOW);
            c++;
            st=0;
          }
        } else {
          digitalWrite(matrice.pinsRows[r],HIGH); //on Ã©teint
          digitalWrite(matrice.pinsCols[c],LOW); //et on place Ã  l'Ã©tat HIGH pour la suivante
          c++;
        }
      }else{
        c=0;
        r++;
      }
    }else{
      r=0;
      c=0;
    }
  }
  else {
    r=0;
    c=0;
  }
}




void parse_setup_matrice(JsonVariant json) {
	if(ALLOW_DEBUG) Serial.println("setup new matrix");
	if(!json.is<JsonObject>()) {
		return;
	}
	parse_destroy_matrix();
	
	JsonObject object = json.as<JsonObject>();
	
	

	
	for (JsonObject::iterator kv = object.begin(); kv != object.end(); ++kv) {
		if(!kv->value().is<JsonObject>()) {
			if(ALLOW_DEBUG) Serial.print ("matrice setup MUST be a JSON object. Exiting.");
			continue;
		}
		JsonObject matriceConfiguration = kv->value();

    // Display name
		char* name = strdup(kv->key().c_str());

		unsigned char rows = 0;
		if(matriceConfiguration.containsKey("rows"))
			rows = matriceConfiguration["rows"].as<unsigned char>();
    // Max speed
		unsigned char cols = 0;
		if(matriceConfiguration.containsKey("cols"))
			cols = matriceConfiguration["cols"].as<unsigned char>();
		unsigned char pinStart = 0;
		if(matriceConfiguration.containsKey("pinStart"))
			pinStart = matriceConfiguration["pinStart"].as<unsigned char>();

		add_matrice(name, rows, cols, pinStart);

	}

	

}
