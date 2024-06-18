#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL6JQU7WeSu"
#define BLYNK_TEMPLATE_NAME "SmartHome"
#define BLYNK_AUTH_TOKEN "HYcy3beYA14nRCReUmvRWJ2J3eBHlquh"
// Khai báo thư viện
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>
#include <DHT.h>
#include <ESP32Servo.h>
// Lấy ngày giờ
const char* ntpServer1 = "pool.ntp.org";
const char* ntpServer2 = "time.nist.gov";
const long  gmtOffset_sec = 25200;
const int   daylightOffset_sec = 25200;
const char* time_zone = "CET-1CEST,M3.5.0,M10.5.0/3"; 
// Khai báo các chân
#define SW2 33 // chân của nút 1
#define SW1 13 // chân của nút 2
#define DHTPIN 4 // Chân của ĐHT11
#define DHTTYPE DHT11 // Loại DHT11
#define RELAY_1 14 // Chân đèn
#define RELAY_2 27 // Chân KHóa
#define GAS 34 // Chân cảm biến gas
#define bao_gas 5 // Chân còi báo gas
#define MUA 12 // Chân cảm biến mưa
//
DHT dht(DHTPIN, DHTTYPE);
// Đây là khai báo 2 chân SCL, SDA của màn OLED
Adafruit_SH1106 display(21, 22);
// Tạo 1 cái servo có tên là servo
Servo servo;
const int servoPin = 18; // Chân servo
// Id của BLYNK và tên, mật khẩu wifi
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "LDT";
char pass[] = "88888888";
// Khai báo các biến
BlynkTimer timer;
int buttonStateSW1 = LOW;         // Trạng thái nút SW1 trước đó
int relay1State = LOW;            // Trạng thái RELAY_1
int buttonCounterSW1 = 0;         // Biến đếm số lần nút SW1 được nhấn

int buttonStateSW2 = LOW;         // Trạng thái nút SW2 trước đó
int relay2State = LOW;            // Trạng thái RELAY_2
int buttonCounterSW2 = 0; 

//Đọc dữ liệu mà Blynk pin V0 ( cài pin ở phần tạo template của blynk)
BLYNK_WRITE(V0) {
  relay1State = param.asInt();
  digitalWrite(RELAY_1, relay1State);
}
//Đọc dữ liệu mà Blynk pin V3 ( cài pin ở phần tạo template của blynk)
BLYNK_WRITE(V3) {
  relay2State = param.asInt();
  digitalWrite(RELAY_2, relay2State);
}
// Hàm Hiển thị ngày giờ hàm này sẽ gọi vở loop()
void printLocalTime() {
  struct tm timeinfo; // Cấu trúc thời gian %H:%M:%S%d/%m/%Y giờ phút giây ngày tháng năm
  if (!getLocalTime(&timeinfo)) {
    return;
  }

  display.setTextSize(2);
  display.setCursor(17, 28);
  display.println(&timeinfo, "%H:%M:%S");
  Serial.println(&timeinfo, "%H:%M:%S");

  display.setTextSize(1.5);
  display.setCursor(33, 50);
  display.println(&timeinfo, "%d/%m/%Y");
  Serial.println(&timeinfo, "%d/%m/%Y");
  display.display();
}
// hàm setup
void setup() {
  // Cài chân IN OUT
  pinMode(RELAY_1, OUTPUT);
  pinMode(RELAY_2, OUTPUT);
  pinMode(bao_gas, OUTPUT);
  pinMode(GAS, INPUT);
  pinMode(MUA, INPUT);
  pinMode(SW1, INPUT);
  pinMode(SW2, INPUT);
  // 
  servo.attach(servoPin, 500, 2400);
  Serial.begin(9600);
  // Khởi tạo Blynk và cảm biến DHT
  dht.begin();
  Blynk.begin(auth, ssid, pass);
  // Khởi tạo OLED
  display.begin(SH1106_SWITCHCAPVCC, 0x3C);
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);
  display.clearDisplay();
  display.setTextColor(WHITE);
}
// Vòng loop()
void loop() {
  Blynk.run();
  timer.run();

  // khai báo biến
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  // Kiểm tra xem đọc được cảm biến DHT không
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
  }
  // Kiểm tra mưa hay không
  if (digitalRead(MUA) == 0) {
    servo.write(90);
    Serial.println("Co MUA");
  } else {
    servo.write(0);
    Serial.println(" K Co MUA");
  }
  // Kiểm tra có khí gas không
  if (analogRead(GAS) >= 600) {
    digitalWrite(bao_gas, HIGH);
  }
  if (analogRead(GAS) < 600) {
    digitalWrite(bao_gas, LOW);
  }
  // Ghi dữ liệu nhiệt độ độ ẩm lên Blynk
  Blynk.virtualWrite(V1, t);
  Blynk.virtualWrite(V2, h);
  // Hiển thị Nhiệt độ lên oled
  display.clearDisplay();
  display.setTextSize(1.5);
  display.setCursor(0, 0);
  display.print("Nhiet Do: ");
  display.setTextSize(1.5);
  display.setCursor(60, 0);
  display.print(t);
  display.print(" ");
  display.setTextSize(1);
  display.cp437(true);
  display.write(167);
  display.setTextSize(1.5);
  display.print("C");
// Hiển thị độ ẩm lên oled
  display.setTextSize(1.5);
  display.setCursor(0, 13);
  display.print("Do Am: ");
  display.setTextSize(1.5);
  display.setCursor(40, 13);
  display.print(h);
  display.print(" %");
// Hiển thị Giờ lên OLED
  printLocalTime();
  //
  checkButtonSW1();
  checkButtonSW2();
}
void checkButtonSW1() {
  int newButtonStateSW1 = digitalRead(SW1);
  
  // Kiểm tra xem nút SW1 có thay đổi trạng thái hay không
  if (newButtonStateSW1 != buttonStateSW1) {
    // Nếu có sự thay đổi, chờ một khoảng thời gian nhất định để đảm bảo không có nhiễu
    delay(50);
    newButtonStateSW1 = digitalRead(SW1);
    
    // Kiểm tra lại trạng thái nút SW1
    if (newButtonStateSW1 != buttonStateSW1) {
      // Nếu trạng thái mới khác trạng thái trước đó
      if (newButtonStateSW1 == HIGH) {
        // Nút SW1 được nhấn, tăng biến đếm
        buttonCounterSW1++;
        // Đảo trạng thái RELAY_1
        relay1State = (buttonCounterSW1 % 2 == 1) ? HIGH : LOW;
        digitalWrite(RELAY_1, relay1State);
        // Gửi trạng thái RELAY_1 lên Blynk
        Blynk.virtualWrite(V0, relay1State);
        delay(100);
      }
    }
  }
  // Cập nhật trạng thái nút SW1
  buttonStateSW1 = newButtonStateSW1;
}
void checkButtonSW2() {
  int newButtonStateSW2 = digitalRead(SW2);

  // Kiểm tra xem nút SW2 có thay đổi trạng thái hay không
  if (newButtonStateSW2 != buttonStateSW2) {
    // Nếu có sự thay đổi, chờ một khoảng thời gian nhất định để đảm bảo không có nhiễu
    delay(50);
    newButtonStateSW2 = digitalRead(SW2);

    // Kiểm tra lại trạng thái nút SW2
    if (newButtonStateSW2 != buttonStateSW2) {
      // Nếu trạng thái mới khác trạng thái trước đó
      if (newButtonStateSW2 == HIGH) {
        // Nút SW2 được nhấn, tăng biến đếm
        buttonCounterSW2++;
        // Đảo trạng thái RELAY_2
        relay2State = (buttonCounterSW2 % 2 == 1) ? HIGH : LOW;
        digitalWrite(RELAY_2, relay2State);
        // Gửi trạng thái RELAY_2 lên Blynk
        Blynk.virtualWrite(V3, relay2State);
        delay(100);
      }
    }
  }
  // Cập nhật trạng thái nút SW2
  buttonStateSW2 = newButtonStateSW2;
}