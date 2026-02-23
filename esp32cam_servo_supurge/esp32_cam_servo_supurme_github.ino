#include <ESP32Servo.h>

Servo camServo;

static const int SERVO_PIN = 14;   // IO14
static const int MIN_US = 500;
static const int MAX_US = 2500;

void setup() {
  Serial.begin(115200);

  camServo.setPeriodHertz(50);
  camServo.attach(SERVO_PIN, MIN_US, MAX_US);

  Serial.println("Servo test basladi.");
}

void loop() {
  for (int a = 0; a <= 180; a += 5) {
    camServo.write(a);
    delay(150);
  }
  for (int a = 180; a >= 0; a -= 5) {
    camServo.write(a);
    delay(150);
  }
}
