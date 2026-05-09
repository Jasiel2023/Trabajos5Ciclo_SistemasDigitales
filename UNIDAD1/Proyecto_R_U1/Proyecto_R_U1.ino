#include <LiquidCrystal_I2C.h>


LiquidCrystal_I2C lcd(0x27, 16, 2);

const int pin_rele = 8; 


// Definicion de  los estados 
enum Estados { ESPERANDO, REGANDO, PAUSA_ABSORCION };
Estados estadoActual = ESPERANDO;

unsigned long cronometro = 0;
const unsigned long TIEMPO_RIEGO = 3000;   
const unsigned long TIEMPO_PAUSA = 5000;   

void setup() {
  Wire.begin();        
  lcd.init();          
  lcd.backlight();     
  
  pinMode(pin_rele, OUTPUT);
  
  digitalWrite(pin_rele, HIGH); 

  Serial.begin(9600); 
}

void loop() {
  //Mapeamos 
  int lecturaCruda = analogRead(A0);
  int porcentaje = map(lecturaCruda, 890, 310, 0, 100);
  porcentaje = constrain(porcentaje, 0, 100); 
  
  unsigned long tiempoActual = millis();

  mostrarLCD(porcentaje);
// Control de Estados
  switch (estadoActual) {
    
    case ESPERANDO:
      if (porcentaje < 30) {
        // Para ENCENDER la bomba
        digitalWrite(pin_rele, LOW);
        cronometro = tiempoActual;
        estadoActual = REGANDO;
      }
      break;

    case REGANDO:
      if (tiempoActual - cronometro >= TIEMPO_RIEGO) {
        // Para APAGAR la bomba
        digitalWrite(pin_rele, HIGH);
        cronometro = tiempoActual; 
        estadoActual = PAUSA_ABSORCION; 
      }
      break;

    case PAUSA_ABSORCION:
      // Esperamos 5 segundos a que el agua filtre en la tierra
      if (tiempoActual - cronometro >= TIEMPO_PAUSA) {
        estadoActual = ESPERANDO;
      }
      break;
  }
}

void mostrarLCD(int valor) {
  lcd.setCursor(0, 0);
  lcd.print("Humedad: ");
  lcd.print(valor);
  lcd.print("%   ");
}
