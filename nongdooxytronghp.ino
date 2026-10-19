#define BLYNK_TEMPLATE_ID   "TMPL6FHityvyw"
#define BLYNK_TEMPLATE_NAME "oxytestmax301002"
#define BLYNK_AUTH_TOKEN    "mFphZ9U__6vHjnYplN0de55J7Ek_-XpT"

#include <Wire.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ================= WIFI =================
char ssid[] = "B Khai T.5";
char pass[] = "0961614809";

// ================= NÚT NHẤN VẬT LÝ =================
#define BUTTON_PIN 14

// ================= OLED =================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ================= MAX30102 =================
MAX30105 particleSensor;

// I2C ESP32
#define SDA_PIN 21
#define SCL_PIN 22

// Số mẫu đo (100 mẫu @ 25 sps = 4 giây)
#define BUFFER_SIZE 100

// Số mẫu mới trước mỗi lần tính lại (25 mẫu = ~1 giây)
#define STEP 25

#define FINGER_THRESHOLD 50000

uint32_t irBuffer[BUFFER_SIZE];
uint32_t redBuffer[BUFFER_SIZE];

int32_t spo2;
int8_t validSpO2;

int32_t heartRate;        // thuật toán bắt buộc phải có, không hiển thị
int8_t validHeartRate;

// Trạng thái cửa sổ trượt
int filled = 0;
int newCount = 0;
bool noFingerShown = false;

// Trạng thái bật/tắt hệ thống (web V8 + nút vật lý)
bool systemOn = true;


// ================= HÀM PHỤ =================
void showNoFinger()
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("DO NONG DO OXY");
  display.println();
  display.println("Dat ngon tay");
  display.println("len MAX30102");
  display.display();
}

void showMeasuring()
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("HE THONG DO SpO2");
  display.println();
  display.println("Dang do...");
  display.println("Giu yen ngon tay");
  display.display();
}

void showSystemOff()
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("HE THONG DO SpO2");
  display.println();
  display.println("HE THONG DA TAT");
  display.println("Nhan nut hoac Blynk");
  display.println("de bat lai");
  display.display();
}

// Áp dụng trạng thái bật/tắt
void applySystemState()
{
  filled = 0;
  newCount = 0;

  if (systemOn)
  {
    particleSensor.wakeUp();
    particleSensor.clearFIFO();
    showNoFinger();
    noFingerShown = true;
    Serial.println("HE THONG: BAT");
  }
  else
  {
    particleSensor.shutDown();
    showSystemOff();
    Serial.println("HE THONG: TAT");
  }
}

// Nút nhấn vật lý: mỗi lần nhấn đảo trạng thái và đồng bộ lên web
void handleButton()
{
  static bool lastState = HIGH;
  static unsigned long lastChange = 0;

  bool s = digitalRead(BUTTON_PIN);

  if (s != lastState && millis() - lastChange > 50)   // chống dội phím 50 ms
  {
    lastChange = millis();
    lastState = s;
if (s == LOW)                                     // vừa nhấn xuống
    {
      systemOn = !systemOn;
      applySystemState();

      if (Blynk.connected())
      {
        Blynk.virtualWrite(V8, systemOn ? 1 : 0);     // cập nhật Switch trên web
      }
    }
  }
}


// ================= BLYNK =================
BLYNK_CONNECTED()
{
  Blynk.syncVirtual(V8);     // lấy lại trạng thái bật/tắt đã lưu
}

BLYNK_WRITE(V8)
{
  systemOn = (param.asInt() == 1);
  applySystemState();
}


// ================= SETUP =================
void setup()
{
  Serial.begin(115200);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  Wire.begin(SDA_PIN, SCL_PIN);

  // ================= OLED =================
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR))
  {
    Serial.println("LOI: Khong tim thay OLED!");
    while (1);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("HE THONG DO SpO2");
  display.println();
  display.println("Khoi dong...");
  display.display();

  delay(1000);

  // ================= MAX30102 =================
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST))
  {
    Serial.println("LOI: Khong tim thay MAX30102!");

    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("LOI MAX30102!");
    display.println();
    display.println("Kiem tra:");
    display.println("SDA -> GPIO21");
    display.println("SCL -> GPIO22");
    display.println("VCC -> 3.3V");
    display.println("GND -> GND");
    display.display();

    while (1);
  }

  Serial.println("MAX30102 OK");

  // sampleAverage = 4, sampleRate = 100 => 25 sps thực tế
  particleSensor.setup(60, 4, 2, 100, 411, 4096);

  // Cường độ LED
  particleSensor.setPulseAmplitudeRed(0x2A);
  particleSensor.setPulseAmplitudeIR(0x2A);

  // ================= BLYNK =================
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Dang ket noi WiFi");
  display.println("va Blynk...");
  display.display();

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  Serial.println("Dat ngon tay len MAX30102");

  showNoFinger();
  noFingerShown = true;
}


// ================= LOOP =================
void loop()
{
  Blynk.run();
  handleButton();

  // ---------- Hệ thống đang tắt: chỉ giữ kết nối Blynk và nghe nút nhấn ----------
  if (!systemOn)
  {
    delay(20);
    return;
  }

  // ---------- Đọc 1 mẫu mới ----------
  while (particleSensor.available() == false)
  {
    particleSensor.check();
    Blynk.run();
    handleButton();
    if (!systemOn) return;     // vừa bị tắt (từ web hoặc nút vật lý)
  }

  uint32_t red = particleSensor.getRed();
  uint32_t ir  = particleSensor.getIR();
  particleSensor.nextSample();

  // ---------- Kiểm tra ngón tay (từng mẫu) ----------
  if (ir < FINGER_THRESHOLD)
  {
    filled = 0;
    newCount = 0;
if (!noFingerShown)
    {
      Serial.println("Khong phat hien ngon tay!");
      Serial.println("SpO2: --");
      showNoFinger();
      noFingerShown = true;
    }
    return;
  }

  if (noFingerShown)
  {
    noFingerShown = false;
    showMeasuring();
  }

  // ---------- Cửa sổ trượt ----------
  if (filled < BUFFER_SIZE)
  {
    // Lần đầu: thu đủ 100 mẫu
    redBuffer[filled] = red;
    irBuffer[filled]  = ir;
    filled++;

    if (filled < BUFFER_SIZE) return;
  }
  else
  {
    // Đã đủ: dịch 1 mẫu, thêm mẫu mới vào cuối
    memmove(redBuffer, redBuffer + 1, (BUFFER_SIZE - 1) * sizeof(uint32_t));
    memmove(irBuffer,  irBuffer + 1,  (BUFFER_SIZE - 1) * sizeof(uint32_t));
    redBuffer[BUFFER_SIZE - 1] = red;
    irBuffer[BUFFER_SIZE - 1]  = ir;

    if (++newCount < STEP) return;
  }

  newCount = 0;

  // ---------- Tính SpO2 ----------
  maxim_heart_rate_and_oxygen_saturation(
    irBuffer,
    BUFFER_SIZE,
    redBuffer,
    &spo2,
    &validSpO2,
    &heartRate,
    &validHeartRate
  );

  bool spo2Ok = (validSpO2 && spo2 >= 70 && spo2 <= 100);

  // ---------- Serial ----------
  Serial.print("SpO2: ");
  if (spo2Ok) { Serial.print(spo2); Serial.println(" %"); }
  else        { Serial.println("--"); }

  // ---------- Gửi lên Blynk ----------
  if (spo2Ok) Blynk.virtualWrite(V0, spo2);

  // ---------- OLED ----------
  display.clearDisplay();

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("HE THONG DO SpO2");

  display.setTextSize(2);
  display.setCursor(0, 22);
  display.print("SpO2: ");

  if (spo2Ok) { display.print(spo2); display.print("%"); }
  else        { display.print("--"); }

  display.display();
}
