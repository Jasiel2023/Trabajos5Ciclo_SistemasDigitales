const int pin555 = 2; 
const int pinFF = 3; 

// Estructura de datos para cada canal
struct DatosSenal {
  int pin;
  int estadoAnterior;
  unsigned long tiempoAnteriorFlanco;
  unsigned long tiempoCambio;
  unsigned long periodo;
  unsigned long tHigh;
  unsigned long tLow;
  float frecuencia;
  float dutyCycle;
};

// Inicializamos los canales
DatosSenal canal1 = {pin555, LOW, 0, 0, 0, 0, 0, 0.0, 0.0};
DatosSenal canal2 = {pinFF, LOW, 0, 0, 0, 0, 0, 0.0, 0.0};

unsigned long ultimoPrint = 0;

void setup() {
  Serial.begin(9600);
  pinMode(canal1.pin, INPUT);
  pinMode(canal2.pin, INPUT);
}

bool detectarFlancoAscendente(int estadoActual, int estadoAnterior);
void medirPeriodo(DatosSenal &senal, unsigned long tiempoActual);
void calcularFrecuencia(DatosSenal &senal);
void medirDutyCycle(DatosSenal &senal, int estadoActual, unsigned long tiempoActual);
void procesarCanal(DatosSenal &senal);

void loop() {
  procesarCanal(canal1);
  procesarCanal(canal2);

  if (millis() - ultimoPrint >= 1000) {
    ultimoPrint = millis();

    Serial.print("555 -> Frec: ");
    Serial.print(canal1.frecuencia);
    Serial.print(" Hz | Duty: ");
    Serial.print(canal1.dutyCycle);
    Serial.print(" %   ||   ");

    Serial.print("FF -> Frec: ");
    Serial.print(canal2.frecuencia);
    Serial.print(" Hz | Duty: ");
    Serial.print(canal2.dutyCycle);
    Serial.println(" %");
  }
  int estado555 = digitalRead(pin555);
  int estadoFF = digitalRead(pinFF);

  
  Serial.print(estado555);
  Serial.print(","); 
  Serial.println(estadoFF + 2); 
}


// ALGORITMO 1: Detección de flanco 

bool detectarFlancoAscendente(int estadoActual, int estadoAnterior) {
  if (estadoActual == HIGH && estadoAnterior == LOW) {
    return true; 
  }
  return false;  
}


// ALGORITMO 2: Medición de periodo
void medirPeriodo(DatosSenal &senal, unsigned long tiempoActual) {
  senal.periodo = tiempoActual - senal.tiempoAnteriorFlanco;
  senal.tiempoAnteriorFlanco = tiempoActual;
}


// ALGORITMO 3: Cálculo de frecuencia
void calcularFrecuencia(DatosSenal &senal) {
  if (senal.periodo > 0) {
    senal.frecuencia = 1000.0 / (float)senal.periodo;
  }
}


// ALGORITMO 4: Medición de duty cycle
void medirDutyCycle(DatosSenal &senal, int estadoActual, unsigned long tiempoActual) {
  if (estadoActual != senal.estadoAnterior) { 
    if (estadoActual == HIGH) {
      senal.tLow = tiempoActual - senal.tiempoCambio;
    } else if (estadoActual == LOW) {
      senal.tHigh = tiempoActual - senal.tiempoCambio;
    }
    senal.tiempoCambio = tiempoActual; // Actualizamos el tiempo del último cambio

    // Calculamos el Duty Cycle en porcentaje
    if (senal.tHigh > 0 && senal.tLow > 0) {
      senal.dutyCycle = ((float)senal.tHigh / (float)(senal.tHigh + senal.tLow)) * 100.0;
    }
  }
}


// ALGORITMO 5: Multicanal 
void procesarCanal(DatosSenal &senal) {
  int estadoActual = digitalRead(senal.pin);
  unsigned long tiempoActual = millis();

  
  medirDutyCycle(senal, estadoActual, tiempoActual);

  // 2. Verificamos si hubo un flanco ascendente llamando al Algoritmo 1
  if (detectarFlancoAscendente(estadoActual, senal.estadoAnterior)) {
    // 3. Si hubo flanco, calculamos Periodo y Frecuencia (Algoritmos 2 y 3)
    medirPeriodo(senal, tiempoActual);
    calcularFrecuencia(senal);
  }

  // 4. Finalmente, actualizamos el estado para el siguiente ciclo del loop
  senal.estadoAnterior = estadoActual;
}
