/*
  ESP32-CAM (AI Thinker) - Kamera Stream + 5 Kademeli Servo Kontrol
  URL'ler:
    /            -> basit sayfa (icinde stream)
    /stream      -> MJPEG stream
    /servo/0     -> servo 0 derece
    /servo/45    -> servo 45 derece
    /servo/90    -> servo 90 derece
    /servo/135   -> servo 135 derece
    /servo/180   -> servo 180 derece
*/

#include "esp_camera.h"
#include <WiFi.h>
#include "esp_http_server.h"
#include <ESP32Servo.h>

// ===== Wi-Fi =====
const char* WIFI_SSID = "c******4";  //BURAYA_WIFI_ADINIZI_YAZIN
const char* WIFI_PASS = "7******0";  //BURAYA_WIFI_ŞİFRENİZİ_YAZIN

// ===== SERVO =====
Servo camServo;
const int SERVO_PIN = 14;        // Servo sinyal pini (GPIO14)
// İsterseniz güvenli sınır verebilirsiniz (mekanik zorlanmayı azaltır):
const int SERVO_MIN = 0;
const int SERVO_MAX = 180;

// ===== AI Thinker ESP32-CAM pinleri =====
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// ===== Basit HTML (ana sayfa) =====
static const char PROGMEM INDEX_HTML[] = R"rawliteral(
<!doctype html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1"/>
  <title>ESP32-CAM + Servo</title>
  <style>
    body{font-family:Arial;margin:10px}
    .card{border:1px solid #ddd;border-radius:10px;padding:10px;margin-bottom:10px}
    img{width:100%;max-width:720px;border-radius:10px;border:1px solid #ddd}
    .row{display:flex;gap:10px;flex-wrap:wrap}
    a{display:inline-block;padding:10px 14px;border:1px solid #aaa;border-radius:10px;text-decoration:none;color:#000;background:#f7f7f7}
    a:active{transform:scale(0.98)}
    .small{color:#555;font-size:12px;word-break:break-all}
  </style>
</head>
<body>
  <div class="card">
    <h3>ESP32-CAM Canli Goruntu</h3>
    <div class="small">Stream: /stream</div>
    <img src="/stream" />
  </div>

  <div class="card">
    <h3>Servo (5 Kademe)</h3>
    <div class="row">
      <a href="/servo/0">0°</a>
      <a href="/servo/45">45°</a>
      <a href="/servo/90">90°</a>
      <a href="/servo/135">135°</a>
      <a href="/servo/180">180°</a>
    </div>
    <p class="small">Not: Servo komutlari ayni Wi-Fi agindan calisir.</p>
  </div>
</body>
</html>
)rawliteral";

// ===== MJPEG Stream (ESP-IDF örnek yaklaşımı) =====
#define PART_BOUNDARY "123456789000000000000987654321"
static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

// ===== Yardımcı: açı sınırla =====
int clampAngle(int a) {
  if (a < SERVO_MIN) return SERVO_MIN;
  if (a > SERVO_MAX) return SERVO_MAX;
  return a;
}

// ===== Servo endpoint ortak handler (5 URI bunu kullanacak) =====
static esp_err_t servo_fixed_handler(httpd_req_t *req) {
  // req->uri örnek: "/servo/90"
  String uri = String(req->uri);
  int angle = 90;

  int idx = uri.lastIndexOf('/');
  if (idx >= 0 && idx + 1 < (int)uri.length()) {
    angle = uri.substring(idx + 1).toInt();
  }

  // Sadece 5 açı bekliyoruz; yine de güvenlik:
  angle = clampAngle(angle);

  // Hatalı istekleri de 5 açının en yakınına oturtmak isterseniz:
  // (Bu endpointler zaten sabit olduğu için normalde gerek yok)
  camServo.write(angle);

  char resp[48];
  snprintf(resp, sizeof(resp), "OK:ANGLE=%d", angle);
  httpd_resp_set_type(req, "text/plain");
  return httpd_resp_send(req, resp, strlen(resp));
}

// ===== Ana sayfa handler =====
static esp_err_t index_handler(httpd_req_t *req) {
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, INDEX_HTML, strlen(INDEX_HTML));
}

// ===== Stream handler =====
static esp_err_t stream_handler(httpd_req_t *req) {
  camera_fb_t * fb = NULL;
  esp_err_t res = ESP_OK;
  size_t _jpg_buf_len = 0;
  uint8_t * _jpg_buf = NULL;
  char part_buf[64];

  res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
  if (res != ESP_OK) return res;

  while (true) {
    fb = esp_camera_fb_get();
    if (!fb) {
      return ESP_FAIL;
    }

    if (fb->format != PIXFORMAT_JPEG) {
      bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
      esp_camera_fb_return(fb);
      fb = NULL;
      if (!jpeg_converted) return ESP_FAIL;
    } else {
      _jpg_buf = fb->buf;
      _jpg_buf_len = fb->len;
    }

    if (res == ESP_OK) res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
    if (res == ESP_OK) {
      int hlen = snprintf(part_buf, sizeof(part_buf), _STREAM_PART, (unsigned int)_jpg_buf_len);
      res = httpd_resp_send_chunk(req, part_buf, hlen);
    }
    if (res == ESP_OK) res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);

    if (fb) {
      esp_camera_fb_return(fb);
      fb = NULL;
      _jpg_buf = NULL;
    } else if (_jpg_buf) {
      free(_jpg_buf);
      _jpg_buf = NULL;
    }

    if (res != ESP_OK) break;
  }
  return res;
}

// ===== Sunucuyu başlat =====
void startCameraServer() {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 80;

  httpd_handle_t server = NULL;
  if (httpd_start(&server, &config) == ESP_OK) {

    // /
    httpd_uri_t index_uri = {
      .uri       = "/",
      .method    = HTTP_GET,
      .handler   = index_handler,
      .user_ctx  = NULL
    };
    httpd_register_uri_handler(server, &index_uri);

    // /stream
    httpd_uri_t stream_uri = {
      .uri       = "/stream",
      .method    = HTTP_GET,
      .handler   = stream_handler,
      .user_ctx  = NULL
    };
    httpd_register_uri_handler(server, &stream_uri);

    // 5 sabit servo endpoint
    const char* paths[5] = {"/servo/0", "/servo/45", "/servo/90", "/servo/135", "/servo/180"};
    for (int i = 0; i < 5; i++) {
      httpd_uri_t u = {
        .uri       = paths[i],
        .method    = HTTP_GET,
        .handler   = servo_fixed_handler,
        .user_ctx  = NULL
      };
      httpd_register_uri_handler(server, &u);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);

  // ===== Servo başlat =====
  camServo.setPeriodHertz(50);
  camServo.attach(SERVO_PIN, 500, 2400);
  camServo.write(90); // başlangıç orta

  // ===== Kamera konfig =====
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;
  config.pin_d0       = Y2_GPIO_NUM;
  config.pin_d1       = Y3_GPIO_NUM;
  config.pin_d2       = Y4_GPIO_NUM;
  config.pin_d3       = Y5_GPIO_NUM;
  config.pin_d4       = Y6_GPIO_NUM;
  config.pin_d5       = Y7_GPIO_NUM;
  config.pin_d6       = Y8_GPIO_NUM;
  config.pin_d7       = Y9_GPIO_NUM;
  config.pin_xclk     = XCLK_GPIO_NUM;
  config.pin_pclk     = PCLK_GPIO_NUM;
  config.pin_vsync    = VSYNC_GPIO_NUM;
  config.pin_href     = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn     = PWDN_GPIO_NUM;
  config.pin_reset    = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  // Stabil başlangıç ayarları:
  config.frame_size   = FRAMESIZE_QVGA; // daha hızlı/stabil
  config.jpeg_quality = 12;             // 10-15 arası yeterlidir
  config.fb_count     = 2;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Kamera baslatma hatasi: 0x%x\n", err);
    return;
  }

  // ===== Wi-Fi =====
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  Serial.print("WiFi baglaniyor");
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println("\nBaglandi!");
  Serial.print("ESP32-CAM IP: ");
  Serial.println(WiFi.localIP());

  // ===== Sunucu =====
  startCameraServer();

  Serial.println("Adresler:");
  Serial.println("http://<IP>/");
  Serial.println("http://<IP>/stream");
  Serial.println("http://<IP>/servo/0  /45  /90  /135  /180");
}

void loop() {
  // HTTP sunucu arkaplanda calisir
  delay(50);
}
