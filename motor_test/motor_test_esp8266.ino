// ===== MOTOR BAĞLANTILARI =====

// SAĞ MOTOR
const int ENB = D5;
const int IN3 = D2;
const int IN4 = D1;

// SOL MOTOR
const int ENA = D4;
const int IN1 = D7;
const int IN2 = D6;

void setup() 
{
  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  stopAll();
  delay(1000);
}

void loop() 
{
  // ==== 2 SANİYE İLERİ ====
  forward();
  delay(2000);

  // ==== 2 SANİYE DUR ====
  stopAll();
  delay(2000);

  // ==== 2 SANİYE GERİ ====
  backward();
  delay(2000);

  // ==== 2 SANİYE DUR ====
  stopAll();
  delay(2000);
}

// ====== FONKSİYONLAR ======

void forward() 
{
  // Sağ ileri
  digitalWrite(ENB, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  // Sol ileri
  digitalWrite(ENA, HIGH);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
}

void backward() 
{
  // Sağ geri
  digitalWrite(ENB, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  // Sol geri
  digitalWrite(ENA, HIGH);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
}

void stopAll() 
{
  digitalWrite(ENB, LOW);
  digitalWrite(ENA, LOW);
}
