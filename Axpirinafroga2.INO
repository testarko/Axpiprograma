#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// ===== THINGSBOARD =====
#define TOKEN "r5PkG936BOEcCDtuY0mb"
#define THINGSBOARD_SERVER "thingsboard.cloud"
#define THINGSBOARD_PORT 1883

// ===== WIFI =====
const char* ssid = "OLP";
const char* pasahitza = "oteitzaLP";

// ===== PINAK =====
const int ldrPin = A0;
const int ledPin = 14;  // D5 = GPIO14

// ===== ALDAGAIAK =====
int ldrBalioa = 0;
int thresholdPiztu  = 1010;  // Iluntasunean pizteko threshold
int thresholdItzali = 1010;  // Argitan itzaltzeko threshold
bool ledEgoera = false;

// ===== OBJETUAK =====
WiFiClient espClient;
PubSubClient client(espClient);

// ===== WIFI-ra KONEXIOA =====
void konektatuWiFi() {
  Serial.println();
  Serial.println("WiFi-ra konektatzen...");
  Serial.print("Sarea: ");
  Serial.println(ssid);

  WiFi.begin(ssid, pasahitza);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi-ra konektatuta!");
  Serial.print("IP helbidea: ");
  Serial.println(WiFi.localIP());
}

// ===== THINGSBOARD-era KONEXIOA =====
void konektatuThingsBoard() {
  while (!client.connected()) {
    Serial.print("ThingsBoard-era konektatzen...");

    if (client.connect("NodeMCU", TOKEN, NULL)) {
      Serial.println(" konektatuta!");
    } 
    else {
      Serial.print(" Error: ");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

void setup() {
  pinMode(ledPin, OUTPUT);
  Serial.begin(115200);
  delay(1000); // Serial Monitor prest egon dadin

  konektatuWiFi();
  client.setServer(THINGSBOARD_SERVER, THINGSBOARD_PORT);
}

void loop() {
  if (!client.connected()) {
    konektatuThingsBoard();
  }

  client.loop();

  // ===== LDR BALIOAREN LECTURA =====
  ldrBalioa = analogRead(ldrPin);

  // ===== LED KONTROLA =====
  // Iluntasunean piztu, argitan itzali
  if (ldrBalioa > thresholdPiztu) {
    ledEgoera = true;  // Iluntasuna → piztu
  }
  if (ldrBalioa < thresholdItzali) {
    ledEgoera = false; // Argitasuna → itzali
  }

  digitalWrite(ledPin, ledEgoera);

  // ===== JSON SORTU =====
  String ledTestua = ledEgoera ? "Itzalita" : "Piztuta";
  String payload = "{\"LDR\":" + String(ldrBalioa) + ",\"LED\":\"" + ledTestua + "\"}";

  // ===== THINGSBOARD-era BIDALI =====
  client.publish("v1/devices/me/telemetry", payload.c_str());

  // ===== SERIAL MONITOR =====
  Serial.print("LDR balioa: ");
  Serial.print(ldrBalioa);
  Serial.print(" | LED egoera: ");
  Serial.println(ledTestua);

  delay(1000); // Bidalketa motela ThingsBoard eta Serial Monitorrera
}
