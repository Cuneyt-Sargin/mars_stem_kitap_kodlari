#include <ESP8266WiFi.h>

const char* ssid     = "c******4";     // Bağlanacağın Wi-Fi adını giriniz.
const char* password = "7******0";      // Bağlanacağın Wi-Fi şifresini giriniz.
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("WiFi'ye baglaniyor...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nBağlandı!");
  Serial.print("IP Adresi: ");
  Serial.println(WiFi.localIP());   // Bu komutla IP Seri Ekranda Görüntüleniyor.
}

void loop() {
  // boş – sadece IP almak için kullandığımız için boş kalacak.
}
