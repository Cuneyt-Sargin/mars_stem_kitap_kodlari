#include <WiFi.h>

// ===== Wi-Fi Bilgileri =====
const char* ssid     = "c******4";        // Bağlanılacak Wi-Fi adı
const char* password = "7******0";   // Wi-Fi şifresi

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("WiFi'ye baglaniyor...");
  WiFi.mode(WIFI_STA);        // ESP32-CAM istemci (station) modunda
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nBaglandi!");
  Serial.print("ESP32-CAM IP Adresi: ");
  Serial.println(WiFi.localIP());   //  ESP32-CAM IP BU KOMUTLA SERİ EKRANDA YAZDIRILIYOR.
}

void loop() {
  // Bu uygulamada loop bos birakilmistir.
  // Amac yalnizca IP adresini ogrenmektir.
}
