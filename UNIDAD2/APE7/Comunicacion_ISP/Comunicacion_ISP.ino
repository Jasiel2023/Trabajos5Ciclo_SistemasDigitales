#include <SPI.h> 
#include <SD.h> 
File archivo; 
void setup() { 
    Serial.begin(9600); 
    if (!SD.begin(10)) { 

      Serial.println("Error SD"); 
      return; 
    } 
  Serial.println("SD inicializada"); 
} 
void loop() { 
} 