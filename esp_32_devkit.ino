#include <WiFi.h>
#include <HTTPClient.h>

#define PIR_PIN 13
#define BUZZER_PIN 4
#define LDR1_PIN 32
#define LDR2_PIN 33
#define LDR3_PIN 34

#define THRESHOLD1 1500
#define THRESHOLD2 1500
#define THRESHOLD3 1500
#define BUFFER 100

unsigned long previousMillis = 0;
const long interval = 1000;

// 🧭 WiFi & ESP32-CAM details
const char* ssid = "AAA";
const char* password = "SURYAKumar";
const char* camServer = "http://10.197.81.59";  // 👈 Replace with your ESP32-CAM IP

void setup() {
  Serial.begin(115200);

  pinMode(PIR_PIN, INPUT);
  pinMode(LDR1_PIN, INPUT);
  pinMode(LDR2_PIN, INPUT);
  pinMode(LDR3_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  Serial.println("📡 Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi Connected!");
  Serial.print("📶 IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.println("✅ Ready — Monitoring Motion & Beams\n");
}

void triggerCamera() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = String(camServer) + "/capture";
    http.begin(url);
    int httpCode = http.GET();
    Serial.printf("📷 Triggered camera, response: %d\n", httpCode);
    http.end();
  } else {
    Serial.println("⚠️ WiFi not connected, cannot trigger camera.");
  }
}

void sendEventToCam(String eventType, String message) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = String(camServer) + "/" + eventType + "?msg=" + message;
    http.begin(url);
    int httpCode = http.GET();
    Serial.printf("📡 Sent %s event: %s (response: %d)\n", eventType.c_str(), message.c_str(), httpCode);
    http.end();
  } else {
    Serial.println("⚠️ WiFi not connected, cannot send event.");
  }
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    int pirState = digitalRead(PIR_PIN);
    int ldr1 = analogRead(LDR1_PIN);
    int ldr2 = analogRead(LDR2_PIN);
    int ldr3 = analogRead(LDR3_PIN);

    // 🧭 PIR motion detection
    if (pirState == HIGH) {
      Serial.println("👀 Motion detected!");
      digitalWrite(BUZZER_PIN, HIGH);
      sendEventToCam("pir", "Motion detected");
      triggerCamera();
      delay(500);
      digitalWrite(BUZZER_PIN, LOW);
    }

    // 🔦 Laser beam detection
    if (ldr1 < THRESHOLD1 - BUFFER || ldr2 < THRESHOLD2 - BUFFER || ldr3 < THRESHOLD3 - BUFFER) {
      Serial.println("🔦 Laser beam interrupted!");
      digitalWrite(BUZZER_PIN, HIGH);
      sendEventToCam("laser", "Beam broken");
      triggerCamera();
      delay(500);
      digitalWrite(BUZZER_PIN, LOW);
    }
  }
}
