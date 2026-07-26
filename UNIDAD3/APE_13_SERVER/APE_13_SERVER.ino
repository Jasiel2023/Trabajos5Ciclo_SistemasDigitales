#include <WiFi.h>
#include <WebServer.h>

// Definimos el servidor web en el puerto 80 (HTTP estándar)
WebServer server(80);

const int ledPin = 5; 
bool estadoLed = false;


const char* ssid = "Internet_UNL";
const char* password = "UNL1859WiFi";


unsigned long tiempoPrevioWifi = 0;
const long intervaloWifi = 500; // Revisar o imprimir puntos cada 500 ms
bool wifiConectadoPreviamente = false;

// Función que diseña la página HTML y la envía a la computadora
void handleRoot() {
  // Usamos Raw String Literals para escribir HTML/CSS sin escapar comillas
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Control IoT ESP32</title>
  <style>
    /* Reset y configuración base */
    * {
      box-sizing: border-box;
      margin: 0;
      padding: 0;
      font-family: 'Segoe UI', Roboto, Helvetica, Arial, sans-serif;
    }
    
    /* Fondo con gradiente moderno */
    body {
      background: linear-gradient(135deg, #0f0c29, #302b63, #24243e);
      color: #ffffff;
      min-height: 100vh;
      display: flex;
      justify-content: center;
      align-items: center;
      overflow: hidden;
    }

    /* Tarjeta principal con efecto Glassmorphism (Cristal) */
    .card {
      position: relative; /* Necesario para posicionar la etiqueta flotante */
      background: rgba(255, 255, 255, 0.1);
      backdrop-filter: blur(15px);
      -webkit-backdrop-filter: blur(15px);
      border: 1px solid rgba(255, 255, 255, 0.2);
      border-radius: 24px;
      padding: 40px 50px;
      box-shadow: 0 8px 32px 0 rgba(0, 0, 0, 0.37);
      text-align: center;
      width: 90%;
      max-width: 400px;
      transition: all 0.3s ease;
    }

    /* Etiqueta "Estoy vivo" */
    .alive-badge {
      position: absolute;
      top: 15px;
      right: 15px;
      background: rgba(0, 255, 136, 0.1);
      color: #00ff88;
      padding: 6px 12px;
      border-radius: 20px;
      font-size: 0.75rem;
      font-weight: 600;
      border: 1px solid rgba(0, 255, 136, 0.3);
      display: flex;
      align-items: center;
      gap: 8px;
    }

    .alive-dot {
      width: 8px;
      height: 8px;
      border-radius: 50%;
      background-color: #00ff88;
      box-shadow: 0 0 10px #00ff88;
      animation: heartbeat 2s infinite;
    }
    
    /* Estado de error de conexión */
    .alive-badge.error {
      background: rgba(255, 76, 76, 0.1);
      color: #ff4c4c;
      border-color: rgba(255, 76, 76, 0.3);
    }
    .alive-badge.error .alive-dot {
      background-color: #ff4c4c;
      box-shadow: 0 0 10px #ff4c4c;
      animation: none;
    }

    @keyframes heartbeat {
      0% { transform: scale(1); opacity: 1; }
      50% { transform: scale(1.5); opacity: 0.6; }
      100% { transform: scale(1); opacity: 1; }
    }

    /* Tipografías */
    h1 {
      font-size: 1.8rem;
      font-weight: 700;
      margin-bottom: 8px;
      margin-top: 15px;
      background: linear-gradient(to right, #fff, #a5a5a5);
      -webkit-background-clip: text;
      -webkit-text-fill-color: transparent;
    }

    h2 {
      font-size: 1rem;
      font-weight: 400;
      color: #b3b3b3;
      margin-bottom: 35px;
    }

    /* Caja de estado */
    .status-box {
      background: rgba(0, 0, 0, 0.2);
      border-radius: 16px;
      padding: 20px;
      margin-bottom: 30px;
      display: flex;
      align-items: center;
      justify-content: center;
      gap: 12px;
    }

    .status-text {
      font-size: 1.1rem;
      letter-spacing: 0.5px;
    }

    /* Indicador visual (punto de luz) */
    .dot {
      width: 14px;
      height: 14px;
      border-radius: 50%;
      background-color: #555;
      transition: all 0.3s ease;
    }

    .dot.on {
      background-color: #00ff88;
      box-shadow: 0 0 15px #00ff88, 0 0 30px #00ff88;
      animation: pulse 2s infinite;
    }

    .dot.off {
      background-color: #ff4c4c;
      box-shadow: 0 0 10px rgba(255, 76, 76, 0.5);
    }

    @keyframes pulse {
      0% { box-shadow: 0 0 0 0 rgba(0, 255, 136, 0.7); }
      70% { box-shadow: 0 0 0 15px rgba(0, 255, 136, 0); }
      100% { box-shadow: 0 0 0 0 rgba(0, 255, 136, 0); }
    }

    /* Botones modernos */
    .btn {
      display: inline-flex;
      align-items: center;
      justify-content: center;
      width: 100%;
      padding: 16px 24px;
      font-size: 1.2rem;
      font-weight: 600;
      color: white;
      text-decoration: none;
      border-radius: 14px;
      border: none;
      cursor: pointer;
      transition: transform 0.2s ease, box-shadow 0.2s ease;
      letter-spacing: 1px;
    }

    .btn:hover { transform: translateY(-3px); }
    .btn:active { transform: translateY(1px); }

    .btn-on {
      background: linear-gradient(135deg, #00b09b, #96c93d);
      box-shadow: 0 4px 15px rgba(0, 176, 155, 0.4);
    }
    .btn-on:hover { box-shadow: 0 8px 25px rgba(0, 176, 155, 0.6); }

    .btn-off {
      background: linear-gradient(135deg, #eb3349, #f45c43);
      box-shadow: 0 4px 15px rgba(235, 51, 73, 0.4);
    }
    .btn-off:hover { box-shadow: 0 8px 25px rgba(235, 51, 73, 0.6); }
  </style>
</head>
<body>
  <div class="card">
    <!-- Etiqueta de "Estoy vivo" -->
    <div class="alive-badge" id="alive-badge">
      <div class="alive-dot"></div>
      <span id="alive-text">Hola, estoy vivo</span>
    </div>

    <h1>Servidor Web ESP32</h1>
    <h2>Control mediante Red Local</h2>
    
    <div class="status-box">
      <div class="dot {{DOT_CLASS}}"></div>
      <span class="status-text">El LED está: <strong>{{STATE_TEXT}}</strong></span>
    </div>

    <a href="{{ACTION_URL}}" class="btn {{BTN_CLASS}}">{{BTN_TEXT}}</a>
  </div>

  <script>
    // Función que chequea si el ESP32 sigue respondiendo
    async function checkServer() {
      try {
        let res = await fetch('/ping');
        if(res.ok) {
          let badge = document.getElementById('alive-badge');
          badge.classList.remove('error');
          document.getElementById('alive-text').innerText = 'Hola, estoy vivo';
        }
      } catch (e) {
        let badge = document.getElementById('alive-badge');
        badge.classList.add('error');
        document.getElementById('alive-text').innerText = 'Conexión perdida';
      }
    }
    // Ejecutar cada 4 segundos
    setInterval(checkServer, 4000);
  </script>
</body>
</html>
)rawliteral";

  // Reemplazamos las variables dinámicas según el estado del LED
  if (estadoLed) {
    html.replace("{{DOT_CLASS}}", "on");
    html.replace("{{STATE_TEXT}}", "ENCENDIDO");
    html.replace("{{ACTION_URL}}", "/apagar");
    html.replace("{{BTN_CLASS}}", "btn-off");
    html.replace("{{BTN_TEXT}}", "APAGAR LED");
  } else {
    html.replace("{{DOT_CLASS}}", "off");
    html.replace("{{STATE_TEXT}}", "APAGADO");
    html.replace("{{ACTION_URL}}", "/encender");
    html.replace("{{BTN_CLASS}}", "btn-on");
    html.replace("{{BTN_TEXT}}", "ENCENDER LED");
  }

  server.send(200, "text/html", html);
}

void handleEncender() {
  estadoLed = true;
  digitalWrite(ledPin, HIGH);
  server.sendHeader("Location", "/"); 
  server.send(303);
}

void handleApagar() {
  estadoLed = false;
  digitalWrite(ledPin, LOW);
  server.sendHeader("Location", "/"); 
  server.send(303);
}

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  // Configurar el ESP32 en modo Estación (conectarse a una red existente)
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Conectando al Wi-Fi");
  



  // Rutas del Servidor
  server.on("/", handleRoot);          
  server.on("/encender", handleEncender); 
  server.on("/apagar", handleApagar);
  
 
}

void loop() {
  unsigned long tiempoActual = millis();

  
  if (WiFi.status() == WL_CONNECTED) {
    // Si se acaba de conectar por primera vez en este ciclo
    if (!wifiConectadoPreviamente) {
      Serial.println("\n=====================================");
      Serial.println("¡ESP32 Conectado a la Red con Éxito!");
      Serial.print("Dirección IP asignada: "); 
      Serial.println(WiFi.localIP()); 
      Serial.println("=====================================");
      
      server.begin(); // Encendemos el servidor ahora que hay red
      wifiConectadoPreviamente = true;
    }
    
    // El servidor web procesa peticiones de forma continua y fluida
    server.handleClient(); 
    
  } else {
    // Si no está conectado o se cayó la red
    if (wifiConectadoPreviamente) {
      Serial.println("\n[ALERTA] Se perdió la conexión Wi-Fi. Reintentando...");
      wifiConectadoPreviamente = false;
    }

   
    if (tiempoActual - tiempoPrevioWifi >= intervaloWifi) {
      tiempoPrevioWifi = tiempoActual;
      Serial.print(".");
    }
  }
}