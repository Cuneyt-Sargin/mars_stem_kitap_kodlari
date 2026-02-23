#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Servo.h>

// ===== Wi-Fi =====
const char* ssid = "c******4";
const char* pass = "7******0";

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

// ===== SERVO =====
Servo camServo;

// Servo sinyal pini (boş bir pin seçin)
// Varsayılan: D0 (GPIO16). Çalışmazsa alternatif: GPIO3 (RX) -> SERVO_PIN = 3, Pinlerin farklı görevleri olduğunu unutmayın.
static const int SERVO_PIN = D0;

// SG90 için mikro-saniye sınırları
static const int SERVO_MIN_US = 500;
static const int SERVO_MAX_US = 2500;

int currentAngle = 90;

// ===== MOTOR FONKSİYONLARI =====
void stopAll() {
  analogWrite(ENB, 0);
  analogWrite(ENA, 0);
}

void forward() {
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENB, speedVal);

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, speedVal);
}

void backward() {
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENB, speedVal);

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  analogWrite(ENA, speedVal);
}

void left() {
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  analogWrite(ENB, speedVal);
  analogWrite(ENA, speedVal);
}

void right() {
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENB, speedVal);
  analogWrite(ENA, speedVal);
}

// ===== SERVO =====
int clampAngle(int a) {
  if (a < 0) return 0;
  if (a > 180) return 180;
  return a;
}

void setServoAngle(int a) {
  a = clampAngle(a);
  currentAngle = a;
  camServo.write(a);
}

// ===== HTTP HANDLERLAR =====
void handleRoot() {
  server.send(200, "text/plain",
              "ESP8266 CAR+SERVO API READY\n"
              "CAR:   /fwd /back /left /right /stop\n"
              "SERVO: /servo?angle=0..180\n");
}

void handleFwd()   { forward();  server.send(200, "text/plain", "OK:FWD"); }
void handleBack()  { backward(); server.send(200, "text/plain", "OK:BACK"); }
void handleLeft()  { left();     server.send(200, "text/plain", "OK:LEFT"); }
void handleRight() { right();    server.send(200, "text/plain", "OK:RIGHT"); }
void handleStop()  { stopAll();  server.send(200, "text/plain", "OK:STOP"); }

// Servo tek açı: /servo?angle=90
void handleServo() {
  // Çakışmayı azaltmak için: servo ayarlanırken arabayı durdur
  stopAll();

  if (!server.hasArg("angle")) {
    server.send(400, "text/plain", "ERR: angle missing. Example: /servo?angle=90");
    return;
  }

  int a = server.arg("angle").toInt();
  a = clampAngle(a);
  setServoAngle(a);

  server.send(200, "text/plain", "OK:SERVO angle=" + String(a));
}

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

  // Servo
  camServo.attach(SERVO_PIN, SERVO_MIN_US, SERVO_MAX_US);
  setServoAngle(currentAngle);

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

  server.on("/servo", handleServo);

  server.begin();
  Serial.println("HTTP server basladi");
}

void loop() {
  server.handleClient();
}
