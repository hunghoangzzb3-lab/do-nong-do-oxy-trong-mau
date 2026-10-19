# 🩸 Hệ thống đo nồng độ Oxy trong máu (SpO2) với ESP32 + MAX30102 + Blynk

Dự án đo nồng độ oxy trong máu (SpO2) sử dụng **ESP32**, cảm biến **MAX30102**, hiển thị trên màn hình **OLED SSD1306** và gửi dữ liệu lên **Blynk IoT** để theo dõi từ xa. Hệ thống có thể bật/tắt bằng **nút nhấn vật lý** hoặc **Switch trên Blynk**, hai bên luôn được đồng bộ trạng thái với nhau.

> ⚠️ **Lưu ý:** Đây là dự án học tập / nghiên cứu. Thiết bị **không phải thiết bị y tế** và không dùng để chẩn đoán hay điều trị bệnh.

---

## ✨ Tính năng

- Đo SpO2 theo thời gian thực bằng cảm biến MAX30102 (LED Red + IR).
- Thuật toán **cửa sổ trượt**: tính lại kết quả khoảng **mỗi 1 giây** thay vì phải chờ đủ 4 giây mỗi lần.
- Tự động phát hiện có/không có ngón tay đặt lên cảm biến.
- Lọc kết quả không hợp lệ (chỉ chấp nhận SpO2 trong khoảng 70 – 100 %).
- Hiển thị kết quả trên màn hình OLED 128x64 và Serial Monitor.
- Gửi dữ liệu lên Blynk (Virtual Pin **V0**).
- Bật/tắt hệ thống từ xa qua Blynk (Virtual Pin **V8**) hoặc bằng nút nhấn vật lý, có **chống dội phím 50 ms**.
- Khi tắt hệ thống, cảm biến được đưa vào chế độ ngủ (`shutDown`) để tiết kiệm điện, ESP32 vẫn giữ kết nối Blynk.
- Tự đồng bộ lại trạng thái bật/tắt khi kết nối lại Blynk.

---

## 🧰 Phần cứng

| Linh kiện | Số lượng | Ghi chú |
|---|---|---|
| ESP32 DevKit | 1 | Có WiFi |
| Cảm biến MAX30102 | 1 | Giao tiếp I2C |
| Màn hình OLED SSD1306 128x64 | 1 | Giao tiếp I2C, địa chỉ `0x3C` |
| Nút nhấn | 1 | Loại nhấn nhả |
| Dây nối, breadboard | – | – |

### Sơ đồ nối dây

| Thiết bị | Chân thiết bị | Chân ESP32 |
|---|---|---|
| MAX30102 | VIN / VCC | 3.3V |
| MAX30102 | GND | GND |
| MAX30102 | SDA | GPIO 21 |
| MAX30102 | SCL | GPIO 22 |
| OLED SSD1306 | VCC | 3.3V |
| OLED SSD1306 | GND | GND |
| OLED SSD1306 | SDA | GPIO 21 |
| OLED SSD1306 | SCL | GPIO 22 |
| Nút nhấn | Một chân | GPIO 14 |
| Nút nhấn | Chân còn lại | GND |

> MAX30102 và OLED dùng chung bus I2C (SDA/SCL). Nút nhấn dùng điện trở kéo lên nội (`INPUT_PULLUP`) nên **không cần** điện trở ngoài.

---

## 📦 Thư viện cần cài

Cài qua **Arduino IDE → Library Manager** (hoặc PlatformIO):

| Thư viện | Tác giả |
|---|---|
| `Blynk` | Volodymyr Shymanskyy |
| `SparkFun MAX3010x Pulse and Proximity Sensor Library` | SparkFun |
| `Adafruit GFX Library` | Adafruit |
| `Adafruit SSD1306` | Adafruit |

Ngoài ra cần cài **board ESP32** (`esp32 by Espressif Systems`) trong Boards Manager.

---

## ☁️ Cấu hình Blynk

1. Đăng ký / đăng nhập tại [Blynk Console](https://blynk.cloud).
2. Tạo **Template** mới (Hardware: `ESP32`, Connection: `WiFi`).
3. Thêm các **Datastream**:

| Virtual Pin | Tên | Kiểu dữ liệu | Khoảng giá trị | Chức năng |
|---|---|---|---|---|
| `V0` | SpO2 | Integer | 0 – 100 | Hiển thị nồng độ oxy |
| `V8` | Power | Integer | 0 – 1 | Switch bật/tắt hệ thống |

4. Tạo **Web Dashboard** (và/hoặc Mobile Dashboard) gồm:
   - 1 widget **Gauge** hoặc **Label** gắn với `V0`.
   - 1 widget **Switch** gắn với `V8`.
5. Tạo **Device** từ template, lấy ra `BLYNK_TEMPLATE_ID`, `BLYNK_TEMPLATE_NAME`, `BLYNK_AUTH_TOKEN`.

---

## ⚙️ Cài đặt và nạp code

1. Clone repo:
   ```bash
   git clone https://github.com/<ten-ban>/<ten-repo>.git
   ```
2. Mở file `.ino` bằng Arduino IDE.
3. Sửa phần cấu hình ở đầu file:

   ```cpp
   #define BLYNK_TEMPLATE_ID   "YOUR_TEMPLATE_ID"
   #define BLYNK_TEMPLATE_NAME "YOUR_TEMPLATE_NAME"
   #define BLYNK_AUTH_TOKEN    "YOUR_AUTH_TOKEN"

   char ssid[] = "YOUR_WIFI_NAME";
   char pass[] = "YOUR_WIFI_PASSWORD";
   ```
4. Chọn board **ESP32 Dev Module**, chọn đúng cổng COM.
5. Nạp code và mở **Serial Monitor** ở tốc độ `115200`.

> 🔐 **Bảo mật:** Không commit `BLYNK_AUTH_TOKEN`, SSID và mật khẩu WiFi thật lên GitHub. Hãy dùng giá trị mẫu như trên, hoặc tách ra file `secrets.h` và thêm file đó vào `.gitignore`. Nếu lỡ đã đẩy token lên, hãy **tạo lại token mới** trên Blynk Console.

---

## 🚀 Cách sử dụng

1. Cấp nguồn cho ESP32, chờ thiết bị kết nối WiFi và Blynk.
2. Đặt nhẹ ngón tay lên cảm biến MAX30102 và **giữ yên** khoảng 4 – 5 giây.
3. Kết quả SpO2 hiển thị trên OLED, Serial Monitor và dashboard Blynk, được cập nhật khoảng mỗi 1 giây.
4. Nhấn **nút vật lý** hoặc gạt **Switch V8** trên Blynk để bật/tắt hệ thống.

### Các trạng thái màn hình OLED

| Trạng thái | Nội dung hiển thị |
|---|---|
| Khởi động | `Khoi dong...` |
| Kết nối mạng | `Dang ket noi WiFi va Blynk...` |
| Chưa có ngón tay | `Dat ngon tay len MAX30102` |
| Đang thu mẫu | `Dang do... Giu yen ngon tay` |
| Có kết quả | `SpO2: xx%` (hoặc `--` nếu không hợp lệ) |
| Hệ thống tắt | `HE THONG DA TAT` |
| Lỗi cảm biến | `LOI MAX30102!` kèm gợi ý kiểm tra dây nối |

---

## 🧠 Nguyên lý hoạt động

- Cảm biến được cấu hình: `sampleAverage = 4`, `sampleRate = 100` → tốc độ thực tế **25 mẫu/giây**.
- Bộ đệm gồm **100 mẫu** (≈ 4 giây) cho cả kênh Red và IR (`BUFFER_SIZE`).
- Sau lần thu đầy đầu tiên, mỗi **25 mẫu mới** (`STEP`, ≈ 1 giây) hệ thống dịch bộ đệm và tính lại SpO2 bằng hàm `maxim_heart_rate_and_oxygen_saturation()` của Maxim.
- Ngón tay được xem là **có mặt** khi giá trị IR ≥ `FINGER_THRESHOLD` (`50000`). Nếu IR thấp hơn, bộ đệm bị reset.
- Kết quả chỉ được hiển thị/gửi đi khi `validSpO2 == 1` và nằm trong khoảng **70 – 100 %**.
- Nhịp tim (`heartRate`) vẫn được thuật toán tính nhưng **không hiển thị**.

### Các tham số có thể tinh chỉnh

| Tham số | Mặc định | Ý nghĩa |
|---|---|---|
| `BUFFER_SIZE` | `100` | Số mẫu mỗi lần tính |
| `STEP` | `25` | Số mẫu mới trước khi tính lại |
| `FINGER_THRESHOLD` | `50000` | Ngưỡng IR phát hiện ngón tay |
| `setPulseAmplitudeRed/IR` | `0x2A` | Cường độ LED, tăng nếu tín hiệu yếu |
| `BUTTON_PIN` | `14` | Chân nút nhấn |

---

## 🛠️ Xử lý sự cố

| Hiện tượng | Cách khắc phục |
|---|---|
| `Khong tim thay OLED!` | Kiểm tra dây SDA/SCL, nguồn 3.3V, địa chỉ I2C (`0x3C` hoặc `0x3D`) |
| `Khong tim thay MAX30102!` | Kiểm tra dây nối I2C và nguồn cấp cho cảm biến |
| Luôn hiện `--` | Giữ yên ngón tay, đặt vừa phải (không ép quá mạnh), tránh ánh sáng mạnh chiếu trực tiếp |
| Không phát hiện ngón tay | Tăng cường độ LED hoặc giảm `FINGER_THRESHOLD` |
| Không lên Blynk | Kiểm tra WiFi (cần băng tần 2.4 GHz), Template ID và Auth Token |
| Nút nhấn không phản hồi | Kiểm tra nút đã nối GPIO 14 và GND chưa |

---

## 📁 Cấu trúc repo

```
.
├── oxytestmax301002.ino   # Mã nguồn chính
├── README.md
└── .gitignore
```

---

## 📄 Giấy phép

Phát hành theo giấy phép [MIT](LICENSE). Bạn có thể tự do sử dụng, chỉnh sửa và phân phối lại.

---

## 🙏 Ghi nhận

- [SparkFun MAX3010x Library](https://github.com/sparkfun/SparkFun_MAX3010x_Sensor_Library) và thuật toán SpO2 của Maxim.
- [Adafruit SSD1306](https://github.com/adafruit/Adafruit_SSD1306) và [Adafruit GFX](https://github.com/adafruit/Adafruit-GFX-Library).
- [Blynk IoT](https://blynk.io).
