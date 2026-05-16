#include <LiquidCrystal.h>

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

const int pinSensor = A2; 

const int pinLed_Caliente = 10; 
const int pinLed_Normal= 9;
const int pinLed_Frio=8;

const int caliente = 30; 
const int normal = 20; 
 

unsigned long tiempoAnterior = 0;     
const unsigned long intervalo = 1000;

int valorSensor = 0; 
float temperatura = 0; 

enum Estados { FRIO, NORMAL, CALIENTE };
Estados estadoActual = NORMAL;

void setup() {
  lcd.begin(16, 2);
  
  pinMode(pinLed_Caliente, OUTPUT); 
  pinMode(pinLed_Normal, OUTPUT);
  pinMode(pinLed_Frio, OUTPUT);
  Serial.begin(9600);
}

float leetemp(void); 

void loop() {
  unsigned long tiempoActual = millis();
  if(tiempoActual - tiempoAnterior >= intervalo){
  	tiempoAnterior = tiempoActual;
    temperatura = leetemp();
  	lcd.setCursor(0, 0);
  	lcd.print("Temp: ");
  	lcd.print(temperatura);
  	lcd.print("°");
    Serial.println(temperatura);
  // comparamos el valor de la temperatura 
    //Caliente
		if (temperatura > caliente){
 			 digitalWrite(pinLed_Caliente, HIGH);
 			 digitalWrite(pinLed_Normal, LOW); 
 			 digitalWrite(pinLed_Frio, LOW); 
          	 estadoActual=CALIENTE;
          	 mostarEstado();
        // Normal  	 
 		 }else if(temperatura >= normal && temperatura < caliente){
  				digitalWrite(pinLed_Normal, HIGH);
    			digitalWrite(pinLed_Caliente, LOW); 
  				digitalWrite(pinLed_Frio, LOW);
        	    estadoActual=NORMAL;
          		mostarEstado();
 		 } // Frio
     else
       {
 			   digitalWrite(pinLed_Frio, HIGH); 
    		   digitalWrite(pinLed_Normal, LOW); 
  			   digitalWrite(pinLed_Caliente, LOW);
          	   estadoActual=FRIO;
          	   mostarEstado();
  			}
  }
}

float leetemp(void) {
  int valorSensor=0; 
  valorSensor = analogRead(pinSensor); 
 Serial.println(valorSensor);
                                       
  float celsius = (valorSensor / 1023.0) *500;
  return celsius;
}

void mostarEstado(){
	switch (estadoActual) {

    case NORMAL:
		
      lcd.setCursor(0, 1);
      
      lcd.print("Estado:  NORMAL");
      break;

    case CALIENTE:

   
      lcd.setCursor(0, 1);
     
      lcd.print("Estado:CALIENTE");
      break;

    case FRIO:

      lcd.setCursor(0, 1);
   
      lcd.print("Estado:    FRIO");
      break;
  }
}