/**
 * ====================================================================
 * SISTEMA DE CONTROL DE ASCENSOR CON ESP32 Y FREERTOS
 * ====================================================================
 * 
 * FUNCIONALIDADES:
 * - Control de ascensor con servo motor (4 pisos: 0,1,2,3)
 * - Botones físicos para llamar al ascensor
 * - Cola de peticiones con máximo 4 solicitudes
 * - Algoritmo de planificación por dirección (SCAN)
 * - Pantalla LCD 16x2 para mostrar estado
 * - Sistema multitarea con FreeRTOS
 * 
 * ESTADOS DEL ASCENSOR:
 * - ASCENSO: Subiendo entre pisos
 * - BAJADA: Bajando entre pisos  
 * - PARADA: Se detuvo en un piso solicitado
 * - ABIERTO: Puertas abiertas
 * - CERRADO: Puertas cerrándose
 * - ESPERA: Quieto con peticiones pendientes
 * - INACTIV: Sin peticiones
 * ====================================================================
 */


#include <ESP32Servo.h>      
#include <Wire.h>           
#include <LiquidCrystal_I2C.h>

Servo ascensor;                     
LiquidCrystal_I2C lcd(0x27 , 16, 2); 

const int btnPB = 25;  // Botón piso PB (Piso 0 - Planta Baja)
const int btnP1 = 26;  // Botón piso 1
const int btnP2 = 27;  // Botón piso 2
const int btnP3 = 14;  // Botón piso 3

const int TOTAL_PISOS = 4;                    // 4 pisos: 0, 1, 2, 3
int gradosPiso[TOTAL_PISOS] = {0, 60, 120, 180}; // Ángulos del servo por piso


// COLA DE PETICIONES 
#define MAX_PETICIONES 4        
int colaPeticiones[MAX_PETICIONES]; 
int cantidadPeticiones = 0;     // Contador de peticiones en cola


// VARIABLES DE ESTADO DEL ASCENSOR

volatile int pisoActual = 0;    // Piso donde está actualmente (0-3)
bool subiendo = true;           // Dirección: true=subiendo, false=bajando
bool enMovimiento = false;      // true cuando se está moviendo entre pisos
bool puertasAbiertas = false;    // true cuando puertas están abiertas
bool enParada = false;          // true cuando está detenido en un piso (antes de abrir)


String ultimaFila0 = "";        
String ultimaFila1 = "";       
bool lcdInicializado = false;   


// SEMÁFOROS 

SemaphoreHandle_t xMutexCola;   // Protege el acceso a la cola de peticiones
SemaphoreHandle_t xMutexLCD;    // Protege el acceso a la pantalla LCD


// FUNCIÓN: agregarPeticion

// PROPÓSITO: Agrega un piso a la cola de solicitudes
// PARÁMETRO: piso - Número de piso a solicitar (0-3)
// RETORNO: true si se agregó, false si ya existía o cola llena

bool agregarPeticion(int piso) {
  // Verificar si ya existe en la cola (evita duplicados)
  for (int i = 0; i < cantidadPeticiones; i++) {
    if (colaPeticiones[i] == piso) {
      Serial.print("Peticion duplicada ignorada: Piso ");
      Serial.println(piso);
      return false;
    }
  }
  
  // Verificar si hay espacio en la cola
  if (cantidadPeticiones >= MAX_PETICIONES) {
    Serial.println("ERROR: Cola llena!");
    return false;
  }
  
  // Agregar al final de la cola
  colaPeticiones[cantidadPeticiones] = piso;
  cantidadPeticiones++;
  
  Serial.print("Peticion agregada: Piso ");
  Serial.println(piso);
  return true;
}

// FUNCIÓN: eliminarPeticion
// 
// PROPÓSITO: Elimina un piso de la cola (cuando ya fue atendido)
// PARÁMETRO: piso - Número de piso a eliminar

void eliminarPeticion(int piso) {
  for (int i = 0; i < cantidadPeticiones; i++) {
    if (colaPeticiones[i] == piso) {
      // Desplazar elementos hacia la izquierda (mantener orden FIFO)
      for (int j = i; j < cantidadPeticiones - 1; j++) {
        colaPeticiones[j] = colaPeticiones[j + 1];
      }
      cantidadPeticiones--;
      Serial.print("Peticion atendida: Piso ");
      Serial.println(piso);
      break;
    }
  }
}


// FUNCIÓN: hayPeticionesPendientes

// PROPÓSITO: Verifica si hay solicitudes en espera
// RETORNO: true si hay al menos una petición pendiente

bool hayPeticionesPendientes() {
  return cantidadPeticiones > 0;
}


// FUNCIÓN: obtenerSiguientePisoEnRuta (ALGORITMO SCAN)

// PROPÓSITO: Implementa la planificación por dirección del ascensor
//   - Sigue en la misma dirección mientras haya peticiones adelante
//   - Cambia de dirección solo cuando no hay más en el sentido actual
//   - Optimiza el recorrido atendiendo peticiones en el trayecto
// RETORNO: Siguiente piso a atender, o -1 si no hay peticiones

int obtenerSiguientePisoEnRuta() {
  if (!hayPeticionesPendientes()) return -1;
  
  int pisoArriba = -1;  // Piso más cercano hacia arriba
  int pisoAbajo = -1;   // Piso más cercano hacia abajo
  
  // Buscar la petición más cercana en cada dirección
  for (int i = 0; i < cantidadPeticiones; i++) {
    int piso = colaPeticiones[i];
    
    if (piso > pisoActual) {
      // Piso arriba: buscar el más cercano (menor diferencia)
      if (pisoArriba == -1 || piso < pisoArriba) pisoArriba = piso;
    } else if (piso < pisoActual) {
      // Piso abajo: buscar el más cercano (mayor valor, más cerca desde arriba)
      if (pisoAbajo == -1 || piso > pisoAbajo) pisoAbajo = piso;
    }
  }
  
  // Decisión basada en la dirección actual (mantener dirección mientras sea posible)
  if (subiendo) {
    if (pisoArriba != -1) return pisoArriba;  // Seguir subiendo
    if (pisoAbajo != -1) {                     // No hay más arriba, cambiar a bajar
      subiendo = false;
      return pisoAbajo;
    }
  } else {
    if (pisoAbajo != -1) return pisoAbajo;     // Seguir bajando
    if (pisoArriba != -1) {                    // No hay más abajo, cambiar a subir
      subiendo = true;
      return pisoArriba;
    }
  }
  
  return -1;
}


// FUNCIÓN: obtenerEstadoTexto

// PROPÓSITO: Devuelve el estado textual del ascensor para la LCD
// RETORNO: String con el estado actual

String obtenerEstadoTexto() {
  if (enParada) return "PARADA";        // Detenido antes de abrir
  if (puertasAbiertas) return "ABIERTO"; // Puertas abiertas
  if (enMovimiento) {
    if (subiendo) return "ASCENSO";      // Subiendo
    else return "BAJADA";                // Bajando
  }
  if (hayPeticionesPendientes()) {
    return "ESPERA";                     // Quieto con peticiones
  }
  return "INACTIV";                      // Sin actividad
}


// FUNCIÓN: obtenerDireccionFormato

// PROPÓSITO: Formato visual de la posición y dirección
//   - ^2^ : Subiendo en piso 2
//   - v3v : Bajando en piso 3
//   - >2< : Quieto en piso 2
//   - [2] : Parado/Abierto en piso 2
// RETORNO: String con el formato visual

String obtenerDireccionFormato() {
  if (enParada || puertasAbiertas) {
    return "[" + String(pisoActual) + "]";  
  }
  if (enMovimiento) {
    if (subiendo) return "^" + String(pisoActual) + "^";  
    else return "v" + String(pisoActual) + "v";           
  }
  return ">" + String(pisoActual) + "<"; 
}

String obtenerDestinoTexto() {
  if (!hayPeticionesPendientes()) return "---";
  
  int siguiente = obtenerSiguientePisoEnRuta();
  if (siguiente == -1) return "---";
  
  return "P" + String(siguiente);
}


// FUNCIÓN: actualizarLCD

// PROPÓSITO: Actualiza la pantalla LCD sin causar parpadeo
//   - Solo escribe si el contenido cambió
//   - Formato: Act:^2^ LP:1,3  (fila 0)
//   - Formato: Dest:P3 ASCENSO (fila 1)

void actualizarLCD() {
  xSemaphoreTake(xMutexLCD, portMAX_DELAY);  
  
  
  String fila0 = "Act:" + obtenerDireccionFormato();
  
  // Agregar lista de peticiones
  fila0 += " LP:";
  if (cantidadPeticiones == 0) {
    fila0 += "-";
  } else {
    for (int i = 0; i < cantidadPeticiones; i++) {
      fila0 += String(colaPeticiones[i]);
      if (i < cantidadPeticiones - 1) {
        fila0 += ",";
      }
    }
  }
  
  // Rellenar hasta 16 caracteres
  while (fila0.length() < 16) {
    fila0 += " ";
  }
  
¿
  // FILA 1: Dest:P2 Estado:ASCENSO

  String fila1 = "Dest:" + obtenerDestinoTexto() + " " + obtenerEstadoTexto();
  
  // Asegurar 16 caracteres
  while (fila1.length() < 16) {
    fila1 += " ";
  }
  
  // Solo actualizar si hay cambios
  if (!lcdInicializado || fila0 != ultimaFila0) {
    lcd.setCursor(0, 0);
    lcd.print(fila0);
    ultimaFila0 = fila0;
  }
  
  if (!lcdInicializado || fila1 != ultimaFila1) {
    lcd.setCursor(0, 1);
    lcd.print(fila1);
    ultimaFila1 = fila1;
  }
  
  lcdInicializado = true;
  
  xSemaphoreGive(xMutexLCD);  // Liberar semáforo
}


// TAREA: tareaBotones 

// PROPÓSITO: Lee los botones constantemente y agrega peticiones
// PRIORIDAD: 2 (mayor que las demás para respuesta rápida)

void tareaBotones(void *pvParameters) {
  int ultimoEstado[4] = {HIGH, HIGH, HIGH, HIGH};  // Antirrebote
  int botones[4] = {btnPB, btnP1, btnP2, btnP3};
  int pisos[4] = {0, 1, 2, 3};
  
  while (true) {
    for (int i = 0; i < 4; i++) {
      int lectura = digitalRead(botones[i]);
      
      // Detectar flanco descendente (botón presionado)
      if (lectura == LOW && ultimoEstado[i] == HIGH) {
        xSemaphoreTake(xMutexCola, portMAX_DELAY);  // Tomar semáforo de cola
        
        // Permitir peticiones en CUALQUIER momento
        if (pisos[i] != pisoActual) {
          agregarPeticion(pisos[i]);
        } else {
          Serial.println("Ya en este piso, ignorando");
        }
        
        xSemaphoreGive(xMutexCola);  // Liberar semáforo
        actualizarLCD();  
      }
      ultimoEstado[i] = lectura;
    }
    vTaskDelay(pdMS_TO_TICKS(50));  
  }
}


// TAREA: tareaAscensor (FreeRTOS)

// PROPÓSITO: Controla el movimiento y la lógica principal del ascensor
//   - Obtiene siguiente destino
//   - Mueve el servo entre pisos
//   - Gestiona paradas, apertura/cierre de puertas
//   - Implementa la máquina de estados completa

// TAREA: tareaAscensor (FreeRTOS) - CORREGIDA Y COMPLETA

void tareaAscensor(void *pvParameters) {
  while (true) {
    // Obtener siguiente piso destino (protegido por semáforo)
    xSemaphoreTake(xMutexCola, portMAX_DELAY);
    int siguiente = obtenerSiguientePisoEnRuta();
    xSemaphoreGive(xMutexCola);
    
    // Verificar si hay destino y el ascensor está disponible para moverse
    if (siguiente != -1 && !puertasAbiertas && !enParada && !enMovimiento) {
      
      
      int pisoAnterior = pisoActual; 
      
      //  Avanzar un piso a la vez hacia el destino
      if (siguiente > pisoActual) {
        subiendo = true;
        pisoActual++;
      } else if (siguiente < pisoActual) {
        subiendo = false;
        pisoActual--;
      }
      
      enMovimiento = true;
      actualizarLCD();
      
      
      int anguloOrigen  = gradosPiso[pisoAnterior]; // De dónde sale
      int anguloDestino = gradosPiso[pisoActual];   // A dónde llega
      int velocidad = 25; 
      
      // MOVIMIENTO SUAVECITO
      if (anguloOrigen < anguloDestino) {
        // Subiendo suavemente grado por grado
        for (int pos = anguloOrigen; pos <= anguloDestino; pos++) {
          ascensor.write(pos);
          vTaskDelay(pdMS_TO_TICKS(velocidad)); 
        }
      } else {
        // Bajando suavemente grado por grado
        for (int pos = anguloOrigen; pos >= anguloDestino; pos--) {
          ascensor.write(pos);
          vTaskDelay(pdMS_TO_TICKS(velocidad)); 
        }
      }
      
      //  LLEGADA AL PISO 
      enMovimiento = false; 
      actualizarLCD();
      
    
      xSemaphoreTake(xMutexCola, portMAX_DELAY);
      
      bool deboDetenerme = false;
      // Nos detenemos si es su destino final o si alguien llamó en este piso intermedio
      for (int i = 0; i < cantidadPeticiones; i++) {
        if (colaPeticiones[i] == pisoActual) {
          deboDetenerme = true;
          break;
        }
      }
      
      if (deboDetenerme) {
        // Cambiar estados para simular la parada y apertura de puertas
        enParada = true;
        actualizarLCD();
        vTaskDelay(pdMS_TO_TICKS(1000)); 
        
        enParada = false;
        puertasAbiertas = true;
        actualizarLCD();
        
        // Atender y eliminar la petición de la cola
        eliminarPeticion(pisoActual);
        xSemaphoreGive(xMutexCola); 
        
        vTaskDelay(pdMS_TO_TICKS(3000)); 
        
        puertasAbiertas = false; 
        actualizarLCD();
      } else {
        // Si no hay peticiones para este piso (solo va pasando hacia un piso más alto)
        xSemaphoreGive(xMutexCola);
      }
    }
    
    vTaskDelay(pdMS_TO_TICKS(100));  
  }
}


// TAREA: tareaLCD (FreeRTOS)

// PROPÓSITO: Actualiza la pantalla LCD periódicamente
// PRIORIDAD: 1 (baja, solo visualización)

void tareaLCD(void *pvParameters) {
  while (true) {
    actualizarLCD();
    vTaskDelay(pdMS_TO_TICKS(200));  // Actualizar cada 200ms (NO BLOQUEA)
  }
}


void setup() {
  Serial.begin(115200);
  
  
  pinMode(btnPB, INPUT_PULLUP);
  pinMode(btnP1, INPUT_PULLUP);
  pinMode(btnP2, INPUT_PULLUP);
  pinMode(btnP3, INPUT_PULLUP);
  
  
  ascensor.attach(18);
  ascensor.write(gradosPiso[0]);
  Wire.begin(21, 22);
 
  lcd.init();
  lcd.backlight();
  

  lcd.setCursor(0, 0);
  lcd.print("Ascensor RTOS");
  lcd.setCursor(0, 1);
  lcd.print("Iniciando...");
  
  
  xMutexCola = xSemaphoreCreateMutex();  // Protege la cola de peticiones
  xMutexLCD = xSemaphoreCreateMutex();   // Protege la pantalla LCD
  

  xTaskCreate(tareaBotones, "Botones", 4096, NULL, 2, NULL);  
  xTaskCreate(tareaAscensor, "Ascensor", 4096, NULL, 1, NULL); 
  xTaskCreate(tareaLCD, "LCD", 4096, NULL, 1, NULL);
  Serial.println("Sistema iniciado");
  
  
  actualizarLCD();
}


void loop() {
  
}