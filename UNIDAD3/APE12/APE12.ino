
#include <Arduino.h>

const int ADC_PIN= 34;
const int SERVO_PIN= 13;


const int LEDC_CHANNEL= 0;
const int LEDC_FREQ = 50;        // Frecuencia de 50 Hz para servomotor
const int LEDC_RES = 13;

// --- Parámetros de Mapeo ---
const int ADC_MIN = 0;
const int ADC_MAX = 4095;        // Resolución del ADC de 12 bits
const int DUTY_MIN = 1638;       // Ciclo de trabajo aproximado para 0° (0V)
const int DUTY_MAX = 8191;       // Ciclo de trabajo máximo para 180° (3.3V)

QueueHandle_t xQueueDutyCycle;

// --- Prototipos de Tareas ---
void vTaskADC(void *pvParameters);
void vTaskServo(void *pvParameters);


void setup() {
  Serial.begin(115200);
 
  ledcAttach(SERVO_PIN, LEDC_FREQ, LEDC_RES);

 // 2. Configuración del ADC
  analogReadResolution(12); // Asegura resolución de 12 bits (0-4095)

  // 4. Creación de la Cola de Mensajes (Almacena 1 elemento tipo int)
  xQueueDutyCycle = xQueueCreate(1, sizeof(int));

if (xQueueDutyCycle != NULL) {
    // 5. Creación de Tareas Concurrentes con sus respectivas prioridades
    // Tarea de lectura analógica (Prioridad Mayor = 2)
    xTaskCreatePinnedToCore(
      vTaskADC, 
      "Tarea ADC", 
      2048, 
      NULL, 
      2, 
      NULL, 
      1
    );
    // Tarea de accionamiento del Servomotor (Prioridad Menor = 1)
    xTaskCreatePinnedToCore(
      vTaskServo, 
      "Tarea Servo", 
      2048, 
      NULL, 
      1, 
      NULL, 
      1
    );
  } else {
    Serial.println("Error crítico: No se pudo crear la cola FreeRTOS.");
  }

}


void vTaskADC(void *pvParameters) {
  (void) pvParameters;
  int rawADC = 0;
  int calculatedDuty = 0;

  for (;;) {
    // Lectura del voltaje variable (0V - 3.3V)
    rawADC = analogRead(ADC_PIN);

    // Mapeo lineal: de rango ADC (0-4095) a rango PWM requerido (1638-8191)
    calculatedDuty = map(rawADC, ADC_MIN, ADC_MAX, DUTY_MIN, DUTY_MAX);

    // Impresión en cascada para depuración en tiempo real en el Monitor Serie
    Serial.print("ADC_Read: ");
    Serial.print(rawADC);
    Serial.print(" | Target_Duty: ");
    Serial.println(calculatedDuty);

    // Envío del dato a la cola (sobrescribe si está llena para mantener el dato más reciente)
    xQueueOverwrite(xQueueDutyCycle, &calculatedDuty);

    // Bloqueo de la tarea por 50ms para un comportamiento estable y liberar CPU
    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

/**
 * Tarea encargada de recibir el ciclo de trabajo desde la cola
 * y actualizar físicamente el pulso PWM del servomotor.
 */
void vTaskServo(void *pvParameters) {
  (void) pvParameters;
  int receivedDuty = 0;

  for (;;) {
    // Espera indefinida hasta que llegue un nuevo valor de Duty Cycle a la cola
    if (xQueueReceive(xQueueDutyCycle, &receivedDuty, portMAX_DELAY) == pdTRUE) {
      // Escritura en el periférico LEDC
      ledcWrite(SERVO_PIN, receivedDuty);
    }
  }
}

void loop() {
  

}
