#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// ===== Wi-Fi ===== //Öncelikle cep telefonununuz üzerinden internet paylaşımı açmanız gerekmektedir.
const char* ssid = "c******4"; //Bağlı olduğunuz wifi SSID bilgisini giriniz.
const char* pass = "7******0"; ////Bağlı olduğunuz wifi Şifre bilgisini giriniz.

// ===== MOTOR BAĞLANTILARI =====

// SAĞ MOTOR
const int ENB = D5;   // PWM
const int IN3 = D2;
const int IN4 = D1;

// SOL MOTOR
const int ENA = D4;   // PWM
const int IN1 = D7;
const int IN2 = D6;

ESP8266WebServer server(80);

// Hız (0–1023)
int speedVal = 800;

// ===== MOTOR FONKSİYONLARI =====
void stopAll() {
  analogWrite(ENB, 0);
  analogWrite(ENA, 0);
}

void forward() {
  // Sağ ileri
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENB, speedVal);

  // Sol ileri
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, speedVal);
}

void backward() {
  // Sağ geri
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENB, speedVal);

  // Sol geri
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  analogWrite(ENA, speedVal);
}

void left() {
  // Sağ ileri, sol geri
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  analogWrite(ENB, speedVal);
  analogWrite(ENA, speedVal);
}

void right() {
  // Sol ileri, sağ geri
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENB, speedVal);
  analogWrite(ENA, speedVal);
}

// ===== HTTP HANDLERLAR =====
void handleRoot() {
  server.send(200, "text/plain",
              "ESP8266 CAR API READY\n"
              "/fwd /back /left /right /stop\n");
}

void handleFwd()  { forward();  server.send(200, "text/plain", "OK:FWD"); }
void handleBack() { backward(); server.send(200, "text/plain", "OK:BACK"); }
void handleLeft() { left();     server.send(200, "text/plain", "OK:LEFT"); }
void handleRight(){ right();    server.send(200, "text/plain", "OK:RIGHT"); }
void handleStop() { stopAll();  server.send(200, "text/plain", "OK:STOP"); }

// ===== SETUP =====
void setup() {
  Serial.begin(115200);

  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  stopAll();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  Serial.print("WiFi baglaniyor");
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/fwd", handleFwd);
  server.on("/back", handleBack);
  server.on("/left", handleLeft);
  server.on("/right", handleRight);
  server.on("/stop", handleStop);

  server.begin();
  Serial.println("HTTP server basladi");
}

void loop() {
  server.handleClient();
}
