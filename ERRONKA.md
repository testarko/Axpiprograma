#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <Wire.h>
#include <MPU6050.h>

// WiFi eta MQTT konfigurazioa
const char* ssid = "OLP";
const char* password = "oteitzaLP";
const char* mqtt_server = "thingsboard.cloud";

// MQTT autentifikazioa
const char* mqtt_user = "1u65MaYnWbRZBPEZ8XOz";
const char* mqtt_password = "";

// DHT konfigurazioa
#define DHTPIN 14
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// MPU6050 konfigurazioa
MPU6050 mpu;

// MQTT bezeroa
WiFiClient espClient;
PubSubClient client(espClient);

// MQTT gaiaren konfigurazioa
const char* topic = "v1/devices/me/telemetry";

// Funtzioak
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("WiFi-sarera konektatzen: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi-sarera konektatuta!");
  Serial.print("IP helbidea: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("MQTT zerbitzarira konektatzen...");
    if (client.connect("NodeMCUClient", mqtt_user, mqtt_password)) {
      Serial.println("Konektatuta!");
    } else {
      Serial.print("Hutsegitea, kodea = ");
      Serial.print(client.state());
      Serial.println(" Saiatzen berriro 5 segundo barru...");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);

  dht.begin();
  Wire.begin();

  mpu.initialize(); // MPU6050 hasiera
  if (!mpu.testConnection()) {
    Serial.println("MPU6050 ez da aurkitu!");
    while (1);
  }
  Serial.println("MPU6050 aurkitu eta hasi da.");
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Tenperatura eta hezetasuna irakurri
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Errorea DHT datuak irakurtzean!");
    return;
  }

  // MPU6050 datuak irakurri
  int16_t ax, ay, az;
  mpu.getAcceleration(&ax, &ay, &az);

  // MPU6050-ko irakurketak 'g' unitateetara bihurtu (2G eskala)
  float accelX = ax / 16384.0;
  float accelY = ay / 16384.0;
  float accelZ = az / 16384.0;

  // JSON payload prestatu
  String payload = "{";
  payload += "\"temperatura\":" + String(temperature);
  payload += ",\"hezetasuna\":" + String(humidity);
  payload += ",\"ax\":" + String(accelX);
  payload += ",\"ay\":" + String(accelY);
  payload += ",\"az\":" + String(accelZ);
  payload += "}";

  // MQTT argitalpena
  if (client.publish(topic, payload.c_str())) {
    Serial.print("Datuak argitaratuta: ");
    Serial.println(payload);
  } else {
    Serial.println("Argitalpena huts egin du!");
  }

  delay(5000);
}
