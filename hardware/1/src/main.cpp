#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <driver/i2s.h>
#include <math.h>

// ════════════════════════════════════════
// CẤU HÌNH — THAY ĐỔI THEO MÔI TRƯỜNG
// ════════════════════════════════════════

// --- WiFi ---
const char *ssid = "Ninh Thien";
const char *password = "13092413";

// --- Server ---
// ★ CHẾ ĐỘ 1: Localhost (dev) — dùng HTTP thường
// const char *serverUrl = "http://192.168.0.100:3000/api/hardware-audio";
// const bool useHTTPS = false;

// ★ CHẾ ĐỘ 2: Vercel (production) — dùng HTTPS
const char *serverUrl = "https://YOUR-APP.vercel.app/api/hardware-audio";
const bool useHTTPS = true;

// --- Chân INMP441 (I2S) ---
#define I2S_WS 25
#define I2S_SD 32
#define I2S_SCK 14

#define LED_PIN 2

// --- Audio Buffer ---
// Bộ đệm nhỏ cho mỗi lần đọc I2S (0.25 giây)
#define READ_SAMPLES 4000
int16_t readBuffer[READ_SAMPLES];

// ★ Bộ đệm lớn tích lũy 5 giây trước khi gửi
// 5 giây * 16000 Hz = 80000 mẫu * 2 bytes = 160000 bytes
#define SEND_SECONDS 5
#define SEND_SAMPLES (16000 * SEND_SECONDS)
#define SEND_BUFFER_SIZE (SEND_SAMPLES * 2)
int16_t* sendBuffer = nullptr;
int sendBufferPos = 0; // Vị trí hiện tại (tính bằng sample)

int failCount = 0;

// WiFiClientSecure cho HTTPS
WiFiClientSecure secureClient;

void i2s_install() {
  const i2s_config_t i2s_config = {
      .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
      .sample_rate = 16000,
      .bits_per_sample = i2s_bits_per_sample_t(16),
      .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
      .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_STAND_I2S),
      .intr_alloc_flags = 0,
      .dma_buf_count = 32,
      .dma_buf_len = 1024,
      .use_apll = true};

  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
}

void i2s_setpin() {
  const i2s_pin_config_t pin_config = {.bck_io_num = I2S_SCK,
                                       .ws_io_num = I2S_WS,
                                       .data_out_num = I2S_PIN_NO_CHANGE,
                                       .data_in_num = I2S_SD};

  i2s_set_pin(I2S_NUM_0, &pin_config);
}

// Gửi audio tích lũy lên server
void sendAudioToServer() {
  int bytesToSend = sendBufferPos * 2;
  Serial.printf("[Send] Gui %d bytes (%ds) am thanh...\n", bytesToSend, bytesToSend / 32000);

  // Kiểm tra WiFi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[LOI] Mat ket noi WiFi!");
    digitalWrite(LED_PIN, LOW);
    WiFi.reconnect();
    int retries = 0;
    while (WiFi.status() != WL_CONNECTED && retries < 20) {
      delay(500);
      Serial.print(".");
      retries++;
    }
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("\n[LOI] Khong the ket noi lai!");
      return;
    }
    Serial.println("\nDa ket noi lai WiFi!");
  }

  HTTPClient http;

  if (useHTTPS) {
    http.begin(secureClient, serverUrl);
  } else {
    http.begin(serverUrl);
  }

  http.addHeader("Content-Type", "application/octet-stream");
  http.setTimeout(30000); // 30 giây timeout cho chunk lớn

  int httpCode = http.POST((uint8_t *)sendBuffer, bytesToSend);

  if (httpCode > 0) {
    String response = http.getString();
    Serial.printf("=> OK! HTTP %d | %s\n", httpCode, response.substring(0, 100).c_str());
    failCount = 0;
    digitalWrite(LED_PIN, HIGH);
  } else {
    failCount++;
    Serial.printf("=> [LOI] HTTP %d (lan %d)\n", httpCode, failCount);
    digitalWrite(LED_PIN, LOW);
    delay(100);
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);

    if (failCount >= 5) {
      Serial.println("[CANH BAO] Qua nhieu loi! Thu ket noi lai WiFi...");
      WiFi.disconnect();
      delay(1000);
      WiFi.begin(ssid, password);
      int retries = 0;
      while (WiFi.status() != WL_CONNECTED && retries < 20) {
        delay(500);
        retries++;
      }
      failCount = 0;
    }
  }

  http.end();
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n=============================");
  Serial.println("  LectureSync ESP32 v3.0");
  Serial.println("  (Vercel + Localhost)");
  Serial.println("=============================");

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // ★ Cấp phát bộ đệm lớn
  sendBuffer = (int16_t *)malloc(SEND_BUFFER_SIZE);
  if (!sendBuffer) {
    Serial.println("[LOI] Khong du RAM cho send buffer!");
    while (1) delay(1000);
  }
  Serial.printf("Send buffer: %d bytes allocated\n", SEND_BUFFER_SIZE);
  sendBufferPos = 0;

  // WiFi
  Serial.printf("Dang ket noi WiFi: %s\n", ssid);
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.begin(ssid, password);

  int wifiAttempts = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    wifiAttempts++;
    if (wifiAttempts > 40) {
      Serial.println("\n[LOI] Khong the ket noi WiFi! Khoi dong lai...");
      ESP.restart();
    }
  }

  Serial.println("\nDa ket noi WiFi!");
  Serial.printf("IP: %s\n", WiFi.localIP().toString().c_str());
  Serial.printf("Server: %s\n", serverUrl);
  Serial.printf("HTTPS: %s\n", useHTTPS ? "CO" : "KHONG");

  // ★ HTTPS: Bỏ qua kiểm tra chứng chỉ (đơn giản, an toàn đủ cho ứng dụng này)
  if (useHTTPS) {
    secureClient.setInsecure();
    Serial.println("HTTPS: setInsecure() OK");
  }

  // I2S
  Serial.println("Dang khoi tao I2S INMP441...");
  i2s_install();
  i2s_setpin();
  i2s_start(I2S_NUM_0);
  Serial.println("Micro da san sang!");
  Serial.printf("Che do: tich luy %ds roi gui\n", SEND_SECONDS);
  Serial.println("-----------------------------");

  digitalWrite(LED_PIN, HIGH);
}

void loop() {
  size_t bytesIn = 0;

  // Đọc chunk nhỏ từ micro (0.25 giây)
  esp_err_t result = i2s_read(I2S_NUM_0, readBuffer, sizeof(readBuffer), &bytesIn, portMAX_DELAY);

  if (result != ESP_OK || bytesIn == 0) return;

  int samplesRead = bytesIn / 2;

  // ★ Noise gate + Gain
  int64_t sumSquares = 0;
  for (int i = 0; i < samplesRead; i++) {
    sumSquares += (int64_t)readBuffer[i] * readBuffer[i];
  }
  int16_t rmsLevel = (int16_t)sqrt((double)sumSquares / samplesRead);

  for (int i = 0; i < samplesRead; i++) {
    int32_t sample = readBuffer[i];

    if (rmsLevel < 80) {
      sample = sample / 4;
    } else {
      sample = sample * 6;
    }

    // Soft clipping
    if (sample > 24000) {
      sample = 24000 + (sample - 24000) / 4;
    } else if (sample < -24000) {
      sample = -24000 + (sample + 24000) / 4;
    }
    if (sample > 32767) sample = 32767;
    if (sample < -32768) sample = -32768;

    readBuffer[i] = (int16_t)sample;
  }

  // ★ Tích lũy vào send buffer
  int spaceLeft = SEND_SAMPLES - sendBufferPos;
  int toCopy = min(samplesRead, spaceLeft);
  memcpy(&sendBuffer[sendBufferPos], readBuffer, toCopy * 2);
  sendBufferPos += toCopy;

  // ★ Khi đủ 5 giây → gửi lên server
  if (sendBufferPos >= SEND_SAMPLES) {
    sendAudioToServer();
    sendBufferPos = 0;
  }
}