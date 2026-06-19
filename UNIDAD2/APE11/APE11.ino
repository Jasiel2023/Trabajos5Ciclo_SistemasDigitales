#include <Arduino_FreeRTOS.h>
#include <queue.h>
#include <LiquidCrystal.h>

// LCD: RS, E, D4, D5, D6, D7
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

const int SENSOR_PIN = A0;
const int LED_TX = 6;
const int LED_ALERT = 7;

const int UMBRAL_RUIDO = 300;

struct SensorData {
  int valor;
  unsigned long tiempo;
};

QueueHandle_t rawQueue;
QueueHandle_t eventQueue;

//-------------------------
// TAREA ADQUISICION
//-------------------------
void tareaAdquisicion(void *pvParameters) {

  SensorData dato;

  for (;;) {

    dato.valor = analogRead(SENSOR_PIN);
    dato.tiempo = millis();

    xQueueSend(rawQueue, &dato, 0);

    Serial.print("[T1] Captura: ");
    Serial.println(dato.valor);

    vTaskDelay(pdMS_TO_TICKS(300));
  }
}

//-------------------------
// TAREA PROCESAMIENTO
//-------------------------
void tareaProcesamiento(void *pvParameters) {

  SensorData dato;

  for (;;) {

    if (xQueueReceive(rawQueue, &dato, 0) == pdTRUE) {

      if (dato.valor > UMBRAL_RUIDO) {

        digitalWrite(LED_ALERT, HIGH);
        Serial.println("*** EVENTO CRITICO ***");

      } else {

        digitalWrite(LED_ALERT, LOW);
      }

      xQueueSend(eventQueue, &dato, 0);
    }

    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

//-------------------------
// TAREA COMUNICACION
//-------------------------
void tareaComunicacion(void *pvParameters) {

  SensorData dato;

  for (;;) {

    if (xQueueReceive(eventQueue, &dato, 0) == pdTRUE) {

      unsigned long latencia =
        millis() - dato.tiempo;

      digitalWrite(LED_TX, HIGH);

      Serial.print("[T3] TELEMETRIA -> ");
      Serial.println(dato.valor);

      Serial.print("Latencia: ");
      Serial.print(latencia);
      Serial.println(" ms");

      lcd.setCursor(0, 0);
      lcd.print("Valor:");
      lcd.print(dato.valor);
      lcd.print("    ");

      lcd.setCursor(0, 1);

      if (dato.valor > UMBRAL_RUIDO) {
        lcd.print("RUIDO ALTO    ");
      } else {
        lcd.print("NORMAL        ");
      }

      digitalWrite(LED_TX, LOW);
    }

    vTaskDelay(pdMS_TO_TICKS(200));
  }
}

void setup() {

  Serial.begin(9600);

  pinMode(LED_TX, OUTPUT);
  pinMode(LED_ALERT, OUTPUT);

  lcd.begin(16, 2);

  lcd.setCursor(0, 0);
  lcd.print("Sistema RTOS");

  rawQueue = xQueueCreate(20, sizeof(SensorData));
  eventQueue = xQueueCreate(20, sizeof(SensorData));

  xTaskCreate(
    tareaAdquisicion,
    "Adquisicion",
    128,
    NULL,
    3,
    NULL);

  xTaskCreate(
    tareaProcesamiento,
    "Procesamiento",
    128,
    NULL,
    2,
    NULL);

  xTaskCreate(
    tareaComunicacion,
    "Comunicacion",
    256,
    NULL,
    1,
    NULL);

  vTaskStartScheduler();
}

void loop() {
}