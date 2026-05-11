#include <LiquidCrystal_I2C.h>


LiquidCrystal_I2C lcd(0x27, 16, 2);

const int pin_rele = 8; 


// Definicion de  los estados 
enum Estados { MONITOREANDO, REGANDO, PAUSA_ABSORCION };
Estados estadoActual = MONITOREANDO;
Estados estadoAnterior = MONITOREANDO;
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
  int porcentaje = map(lecturaCruda, 766, 460, 0, 100);
  porcentaje = constrain(porcentaje, 0, 100); 
  
  unsigned long tiempoActual = millis();

  mostrarLCD(porcentaje);
// Control de Estados
  switch (estadoActual) {
    
    case MONITOREANDO:
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
        estadoActual = MONITOREANDO;
      }
      break;
  }
}

void mostrarLCD(int valor) {
  lcd.setCursor(0, 0);
  lcd.print("Humedad: ");
  lcd.print(valor);
  lcd.print("%   ");

  lcd.setCursor(0, 1);
  lcd.print("Est: ");
}


void mostrarEstado(int porcentaje) {

  // Solo limpia cuando cambia de estado
  if (estadoActual != estadoAnterior) {
    lcd.clear();
    estadoAnterior = estadoActual;
  }

  switch (estadoActual) {

    case MONITOREANDO:

      mostrarLCD(porcentaje);

      lcd.setCursor(0, 1);
      lcd.print("MONITOREANDO ");
      break;

    case REGANDO:

      lcd.setCursor(0, 0);
      lcd.print("Sistema Activo");

      lcd.setCursor(0, 1);
      lcd.print("EN RIEGO      ");
      break;

    case PAUSA_ABSORCION:

      mostrarLCD(porcentaje);

      lcd.setCursor(0, 1);
      lcd.print("PAUSA TEMP.   ");
      break;
  }
}
