#include <Servo.h>

Servo kameraServo;

const int SERVO_PIN = D0;   // GPIO16

void setup() 
{
  kameraServo.attach(SERVO_PIN);
}

void loop() 
{
  // 0° → 180°
  for (int aci = 0; aci <= 180; aci += 5) 
  {
    kameraServo.write(aci);
    delay(40);   // Yumuşak hareket için gecikme
  }

  delay(500);

  // 180° → 0°
  for (int aci = 180; aci >= 0; aci -= 5) 
  {
    kameraServo.write(aci);
    delay(40);
  }

  delay(500);
}
