
const int pinBoton = 2;  
const int pinBuzzer = 6;     
const int pinLedRojo = 7;    
const int pinLedVerde = 8;  
const int pinSensor = A0;    


unsigned long tiempoActual = 0;
bool alarmaActiva = false;
bool alarmaSilenciada = false;
float temperaturaActual = 0.0;


struct Tarea {
  int pin;
  unsigned long periodo;     
  unsigned long proximaEjec; 
  int estado;               
};

Tarea tareaHeartbeat = {pinLedVerde, 500, 0, 0};   // Cada medio segundo (500ms) 
Tarea tareaTelemetria = {pinSensor, 2000, 0, 0};  // Cada dos segundos
Tarea tareaAlarma = {pinLedRojo, 300, 0, 0};       // Cada 300 milisegundos 


float leerTemperatura();
void ejecutarHeartbeat(Tarea &t);
void ejecutarTelemetria(Tarea &t);
void ejecutarAlarma(Tarea &t);
void verificarBoton();

void setup() {
  Serial.begin(9600); 
  
  pinMode(pinBoton, INPUT);
  pinMode(pinBuzzer, OUTPUT);
  pinMode(pinLedRojo, OUTPUT);
  pinMode(pinLedVerde, OUTPUT);
  
  Serial.println("Sistema IoT Iniciado - Planificador RTOS Activo"); // Mensaje de inicio [cite: 267]
}

void loop() {
  tiempoActual = millis(); 

  

  // Tarea 1: Ejecución del Heartbeat
  if (tiempoActual >= tareaHeartbeat.proximaEjec) {
    ejecutarHeartbeat(tareaHeartbeat);
    tareaHeartbeat.proximaEjec += tareaHeartbeat.periodo;
  }

  // Tarea 2: Ejecución de Telemetría y Monitoreo Térmico
  if (tiempoActual >= tareaTelemetria.proximaEjec) {
    ejecutarTelemetria(tareaTelemetria);
    tareaTelemetria.proximaEjec += tareaTelemetria.periodo;
  }

  // Tarea 3: Ejecución de la Intermitencia de Alarma
  if (tiempoActual >= tareaAlarma.proximaEjec) {
    ejecutarAlarma(tareaAlarma);
    tareaAlarma.proximaEjec += tareaAlarma.periodo;
  }

  
  verificarBoton();
}


float leerTemperatura() {
  int lectura = analogRead(pinSensor);
  
  float voltaje = lectura * (5.0 / 1024.0); 
  float temperaturaC = (voltaje - 0.5) * 100.0; 
  return temperaturaC;
}

// Tarea 1: Cambia el estado del LED verde
void ejecutarHeartbeat(Tarea &t) {
  t.estado = !t.estado; // Toggle (Invertir estado)
  digitalWrite(t.pin, t.estado);
}

// Tarea 2: Captura lecturas, las envía por Serial y evalúa condiciones lógicas
void ejecutarTelemetria(Tarea &t) {
  temperaturaActual = leerTemperatura(); 

  Serial.print("Temperatura: ");
  Serial.print(temperaturaActual);
  Serial.println(" C");

 
  if (temperaturaActual > 30.0) {
    if (!alarmaActiva) {
      alarmaActiva = true;
      alarmaSilenciada = false; 
    }
  } else {
    // Apagado automático del estado de alarma si la temperatura baja
    alarmaActiva = false;
    alarmaSilenciada = false;
    digitalWrite(pinLedRojo, LOW);
    noTone(pinBuzzer);
  }
}

// Tarea 3: Manejo intermitente y rápido del entorno visual
void ejecutarAlarma(Tarea &t) {
  // Solo actúa si la alarma está encendida en el sistema y no ha sido silenciada por el operador
  if (alarmaActiva && !alarmaSilenciada) {
    t.estado = !t.estado; 
    
    digitalWrite(t.pin, t.estado); 
    
    if (t.estado == 1) {
      tone(pinBuzzer, 1000); 
    } else {
      noTone(pinBuzzer);     
    }
  }
}

// Tarea: Pulsador
void verificarBoton() {
  if (alarmaActiva && !alarmaSilenciada) {
    if (digitalRead(pinBoton) == HIGH) { // Detecta pulsación 
      alarmaSilenciada = true;
      
      // Apagado inmediato 
      digitalWrite(pinLedRojo, LOW);
      noTone(pinBuzzer);
      
      Serial.println("ALERTA: Alarma silenciada manualmente por el operador."); // Registro serial [cite: 278]
    }
  }
}