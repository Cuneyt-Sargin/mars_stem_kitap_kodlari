#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Servo.h>

// ===== Wi-Fi =====
const char* ssid = "c******4";
const char* pass = "7******0";

ESP8266WebServer server(80);

// ===== SERVO =====
Servo camServo;

// Servo sinyal pini
static const int SERVO_PIN = D0;   // GPIO16

// SG90 için mikro-saniye sınırları
static const int SERVO_MIN_US = 500;
static const int SERVO_MAX_US = 2500;

int currentAngle = 90;

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
              "ESP8266 SERVO API READY\n"
              "STEPS: /servo/0 /servo/45 /servo/90 /servo/135 /servo/180\n"
              "QUERY: /servo?angle=0..180\n");
}

// /servo?angle=90
void handleServoQuery() {
  if (!server.hasArg("angle")) {
    server.send(400, "text/plain", "ERR: angle missing. Example: /servo?angle=90");
    return;
  }

  int a = server.arg("angle").toInt();
  setServoAngle(a);

  server.send(200, "text/plain", "OK:SERVO angle=" + String(currentAngle));
}

// 5 kademe
void handleServo0()   { setServoAngle(0);   server.send(200, "text/plain", "OK:SERVO angle=0"); }
void handleServo45()  { setServoAngle(45);  server.send(200, "text/plain", "OK:SERVO angle=45"); }
void handleServo90()  { setServoAngle(90);  server.send(200, "text/plain", "OK:SERVO angle=90"); }
void handleServo135() { setServoAngle(135); server.send(200, "text/plain", "OK:SERVO angle=135"); }
void handleServo180() { setServoAngle(180); server.send(200, "text/plain", "OK:SERVO angle=180"); }

void setup() {
  Serial.begin(115200);

  // Servo
  camServo.attach(SERVO_PIN, SERVO_MIN_US, SERVO_MAX_US);
  setServoAngle(currentAngle);

  // Wi-Fi
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

  // Routes
  server.on("/", handleRoot);

  server.on("/servo", handleServoQuery);

  server.on("/servo/0", handleServo0);
  server.on("/servo/45", handleServo45);
  server.on("/servo/90", handleServo90);
  server.on("/servo/135", handleServo135);
  server.on("/servo/180", handleServo180);

  server.begin();
  Serial.println("HTTP server basladi");
}

void loop() {
  server.handleClient();
}
