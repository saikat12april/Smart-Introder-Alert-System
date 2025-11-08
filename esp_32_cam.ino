#include "esp_camera.h"
#include <WiFi.h>
#include <WebServer.h>
#include "time.h"
#include <base64.h> 

// ---------- WiFi ----------
const char* ssid = "SSID";
const char* password = "WIFI_PASSWORD";

WebServer server(80);

// ---------- Brevo SMTP ---------- 
#define SMTP_HOST "smtp-relay.brevo.com"
#define SMTP_PORT 587//cheak yours
#define SMTP_USER "9b0970001@smtp-brevo.com"//cheak yours
#define SMTP_PASS "xsmtpsib-b2bf13a46aab00e8f9e64d574c9aa48bbb908c26aed57d261c0adf79086f80e6-nSrVmepPIXuYiIaf"//cheak Yours
#define SENDER_EMAIL "sender_email_valid_Brevo"
#define RECIPIENT_EMAIL "Recipient_email_yours"

// ---------- Time ----------
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 19800;    // +5:30 hours
const int daylightOffset_sec = 0;

camera_fb_t * last_fb = NULL;
String lastCaptureTime = "No image captured yet.";

// 🧠 Logs
String pirLog = "";
String laserLog = "";

// ---------- Function: Lightweight Email with photo ----------
void sendEmailWithPhoto_Light() {
  if (!last_fb) {
    Serial.println("⚠️ No photo available for email.");
    return;
  }

  Serial.println("📧 Sending email (lightweight mode)...");

  WiFiClient client;
  if (!client.connect(SMTP_HOST, SMTP_PORT)) {
    Serial.println("❌ SMTP connection failed.");
    return;
  }

  // Encode the captured image into Base64
  String encodedImage = base64::encode(last_fb->buf, last_fb->len);

  // SMTP conversation
  client.println("EHLO esp32");
  delay(200);
  client.println("AUTH LOGIN");
  delay(200);
  client.println(base64::encode(SMTP_USER));
  delay(200);
  client.println(base64::encode(SMTP_PASS));
  delay(200);
  client.println("MAIL FROM:<" + String(SENDER_EMAIL) + ">");
  client.println("RCPT TO:<" + String(RECIPIENT_EMAIL) + ">");
  client.println("DATA");
  client.println("From: ESP32-CAM Security <" + String(SENDER_EMAIL) + ">");
  client.println("To: " + String(RECIPIENT_EMAIL));
  client.println("Subject: ESP32-CAM Alert Photo");
  client.println("MIME-Version: 1.0");
  client.println("Content-Type: multipart/mixed; boundary=frontier");
  client.println();
  client.println("--frontier");
  client.println("Content-Type: text/html");
  client.println();
  client.println("Captured by ESP32-CAM at: " + lastCaptureTime);
  client.println();
  client.println("--frontier");
  client.println("Content-Type: image/jpeg; name=\"photo.jpg\"");
  client.println("Content-Transfer-Encoding: base64");
  client.println("Content-Disposition: attachment; filename=\"photo.jpg\"");
  client.println();
  client.println(encodedImage);
  client.println("--frontier--");
  client.println(".");
  client.println("QUIT");

  Serial.println("✅ Email sent request completed (check inbox).");
  delay(1000);
}

// ---------- Capture Photo ----------
void handleCapture() {
  if (last_fb) {
    esp_camera_fb_return(last_fb);
    last_fb = NULL;
  }
  last_fb = esp_camera_fb_get();
  if (!last_fb) {
    server.send(500, "text/plain", "Camera capture failed");
    return;
  }

  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
    lastCaptureTime = String("📸 Captured at: <b>") + buf + " (IST)</b>";
  } else {
    lastCaptureTime = "⚠️ Time not available (NTP error)";
  }
  Serial.println("📸 Picture captured!");

  // Send email with attached photo
  sendEmailWithPhoto_Light();

  server.send(200, "application/json", "{\"status\":\"ok\"}");
}

void handleLatest() {
  if (!last_fb) {
    server.send(404, "text/plain", "No image yet");
    return;
  }
  server.sendHeader("Content-Type", "image/jpeg");
  server.send_P(200, "image/jpeg", (const char *)last_fb->buf, last_fb->len);
}

void handleGetTime() {
  server.send(200, "text/html", lastCaptureTime);
}

// ---------- PIR Event ----------
void handlePirEvent() {
  if (server.hasArg("msg")) {
    struct tm timeinfo;
    getLocalTime(&timeinfo);
    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
    String entry = "👀 " + server.arg("msg") + " at <b>" + buf + " (IST)</b><br>";
    pirLog += entry;
    Serial.println(entry);
    server.send(200, "text/plain", "PIR event received");
  } else {
    server.send(400, "text/plain", "Missing msg");
  }
}

// ---------- Laser Event ----------
void handleLaserEvent() {
  static int laserCount = 0;
  laserCount++;
  if (server.hasArg("msg")) {
    struct tm timeinfo;
    getLocalTime(&timeinfo);
    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
    String entry = "🔦 [#" + String(laserCount) + "] " + server.arg("msg") + " at <b>" + buf + " (IST)</b><br>";
    laserLog += entry;
    Serial.println(entry);
    server.send(200, "text/plain", "Laser event received");
  } else {
    server.send(400, "text/plain", "Missing msg");
  }
}

void handlePirLog() {
  server.send(200, "text/html", pirLog);
}
void handleLaserLog() {
  server.send(200, "text/html", laserLog);
}

// ---------- Dashboard ----------
void handleRoot() {
  String html = R"rawliteral(
  <html>
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 Security Dashboard</title>
    <style>
      body { background:#111; color:#eee; font-family:Arial; margin:0; overflow-x:hidden; }
      h1 { text-align:center; color:#0ff; }
      .container { display:flex; flex-wrap:wrap; justify-content:center; gap:10px; padding:10px; }
      .section { flex:1 1 300px; border:2px solid #333; border-radius:10px; padding:10px; background:#222; min-height:250px; max-height:400px; overflow-y:auto; scroll-behavior:smooth; }
      img { width:100%; border-radius:10px; }
      .timestamp { color:#0f0; font-weight:bold; margin-top:5px; display:block; }
    </style>
    <script>
      async function refresh() {
        document.getElementById('photo').src = '/latest.jpg?rand=' + Math.random();
        document.getElementById('camtime').innerHTML = await fetch('/getTime').then(r=>r.text());
        document.getElementById('pir').innerHTML = await fetch('/pirLog').then(r=>r.text());
        document.getElementById('laser').innerHTML = await fetch('/laserLog').then(r=>r.text());
        document.getElementById('pir').scrollTop = document.getElementById('pir').scrollHeight;
        document.getElementById('laser').scrollTop = document.getElementById('laser').scrollHeight;
      }
      setInterval(refresh, 4000);
      window.onload = refresh;
    </script>
  </head>
  <body>
    <h1>ESP32 Security Dashboard</h1>
    <div class="container">
      <div class="section">
        <h2>PIR Motion Log</h2>
        <div id="pir">Loading...</div>
      </div>
      <div class="section">
        <h2>Camera Capture</h2>
        <img id="photo" src="/latest.jpg" alt="No image yet">
        <span id="camtime" class="timestamp"></span>
      </div>
      <div class="section">
        <h2>Laser Beam Log</h2>
        <div id="laser">Loading...</div>
      </div>
    </div>
  </body>
  </html>
  )rawliteral";
  server.send(200, "text/html", html);
}

// ---------- Webserver Setup ----------
void startCameraServer() {
  server.on("/", handleRoot);
  server.on("/capture", handleCapture);
  server.on("/latest.jpg", handleLatest);
  server.on("/getTime", handleGetTime);
  server.on("/pir", handlePirEvent);
  server.on("/laser", handleLaserEvent);
  server.on("/pirLog", handlePirLog);
  server.on("/laserLog", handleLaserLog);
  server.begin();
}

// ---------- Setup ----------
void setup() {
  Serial.begin(115200);
  Serial.println("\n🚀 Starting ESP32-CAM...");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi connected!");
  Serial.print("🌐 IP Address: ");
  Serial.println(WiFi.localIP());

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  // 📸 Camera config (AI Thinker)
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = 5;
  config.pin_d1 = 18;
  config.pin_d2 = 19;
  config.pin_d3 = 21;
  config.pin_d4 = 36;
  config.pin_d5 = 39;
  config.pin_d6 = 34;
  config.pin_d7 = 35;
  config.pin_xclk = 0;
  config.pin_pclk = 22;
  config.pin_vsync = 25;
  config.pin_href = 23;
  config.pin_sscb_sda = 26;
  config.pin_sscb_scl = 27;
  config.pin_pwdn = 32;
  config.pin_reset = -1;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_SVGA;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("❌ Camera init failed!");
    return;
  }

  startCameraServer();
  Serial.println("🌐 Web server ready — open your browser to view dashboard.");
}

// ---------- Main Loop ----------
void loop() {
  server.handleClient();
  delay(10);
}
