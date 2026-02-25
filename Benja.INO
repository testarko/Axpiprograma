#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// ======== CONFIGURACIÓN THINGSBOARD ========
#define TOKEN "r5PkG936BOEcCDtuY0mb"
#define THINGSBOARD_SERVER "thingsboard.cloud"
#define THINGSBOARD_PORT 1883

// ======== DATOS WIFI ========
const char* ssid = "OLP";
const char* password = "oteitzaLP";

// ======== OBJETOS ========
WiFiClient espClient;
PubSubClient client(espClient);

// ======== PINES ========
int pinPotentziometroa = A0;
int balioGordina = 0;
int balioEskalatua = 0;
#define PIN_LED 4   // GPIO4 (D2 en NodeMCU)

// =============================
void setup() {
  Serial.begin(115200);

  // Conectar WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  Serial.println("WiFi conectado");

  // Configurar servidor ThingsBoard
  client.setServer(THINGSBOARD_SERVER, THINGSBOARD_PORT);

  // Configurar pin LED como salida
  pinMode(PIN_LED, OUTPUT);
}

// =============================
// Función para reconectar a ThingsBoard
void reconnect() {
  while (!client.connected()) {
    if (client.connect("ESP8266_Client", TOKEN, NULL)) {
      Serial.println("ThingsBoard conectado");
    } else {
      delay(2000);
    }
  }
}

// =============================
// Loop principal
void loop() {
  if (!client.connected()) {
    reconnect();
  }

  client.loop();

  // 1️⃣ Lectura del potenciómetro (0–1023)
  balioGordina = analogRead(pinPotentziometroa);

  // 2️⃣ Escalado a 0–100
  balioEskalatua = map(balioGordina, 0, 1023, 0, 100);
  balioEskalatua = constrain(balioEskalatua, 0, 100);

  // 3️⃣ Control LED físico y virtual
  bool ledState = false;
  if (balioEskalatua > 80) {
    digitalWrite(PIN_LED, HIGH);  // LED físico encendido
    ledState = true;               // LED virtual encendido
  } else {
    digitalWrite(PIN_LED, LOW);   // LED físico apagado
    ledState = false;             // LED virtual apagado
  }

  // 4️⃣ Publicar telemetría a ThingsBoard
  String payload = "{";
  payload += "\"potentziometroa\":";
  payload += balioEskalatua;
  payload += ",\"led\":";
  payload += ledState ? "true" : "false";
  payload += "}";

  client.publish("v1/devices/me/telemetry", payload.c_str());
  Serial.println(payload);

  delay(1000);
}
