#include "12_triac.h"
#include "08_PID.h"
#include "OnOffHighResolutionTimer.h"
#include "AnhLABV01HardWare.h"
#include <Adafruit_MAX31865.h>
#include "SimpleKalmanFilter.h"  // thư viện lọc dữ liệu
#include "Arduino.h"

// Use software SPI: CS, DI, DO, CLK
// Adafruit_MAX31865 ptBuong = Adafruit_MAX31865(PT100_CS1_PIN, PT100_MOSI_PIN, PT100_MISO_PIN, PT100_SCK_PIN);
Adafruit_MAX31865 ptBuong = Adafruit_MAX31865(PT100_CS1_PIN, PT100_MOSI_PIN, PT100_MISO_PIN, PT100_SCK_PIN);
Adafruit_MAX31865 ptCua = Adafruit_MAX31865(PT100_CS2_PIN, PT100_MOSI_PIN, PT100_MISO_PIN, PT100_SCK_PIN);
Adafruit_MAX31865 ptDu = Adafruit_MAX31865(PT100_CS3_PIN, PT100_MOSI_PIN, PT100_MISO_PIN, PT100_SCK_PIN);
SimpleKalmanFilter LocCamBienBuong = SimpleKalmanFilter(1, 1, 2);

//nhiệt
#define tKp 0.01
#define tKi 0.00001
#define tKd 0
#define tKw 0
#define tOutMax 0.15  // *C/s
#define tOutMin -0.015
#define tWindupMax 0.15
#define tWindupMin -0.015
//tốc độ
#define sKp 3000
#define sKi 100
#define sKd 0
#define sKw 0
#define sOutMax 5000  //ms
#define sOutMin 0
#define sWindupMax 5000  //ms
#define sWindupMin 0

#define MAX31865_NHIET_DO_TOI_DA 200.0f
#define MAX31865_DIEN_TRO_THAM_CHIEU 390.0
#define MAX31865_DIEN_TRO_CAM_BIEN 100.0

triac quat((gpio_num_t)TRIAC2_PIN, TIMER_GROUP_0, TIMER_0);
PID pidNhiet(tKp, tKi, tKd, 5000);
PID pidTocDoNhiet(sKp, sKi, sKd, 5000);
HRTOnOffPin Buong(TRIAC1_PIN);

typedef struct saveData {
  float spBuong;
  float spCua;
} data;


int t = 0;
float err;
float speed, currSpeed;
float temp, preTemp;
int thoiGianKichTriac = 0;
bool run = false;

static float fLayNhietDoThucTe(Adafruit_MAX31865& xPt100);
void khoiTaoCamBien();
void khoiTaoQuat();
void khoiTaoTriac();

void setup() {
  Serial.begin(115200);

  khoiTaoCamBien();

  triac::configACDETPIN((gpio_num_t)ACDET_PIN);
  khoiTaoQuat();
  Buong.init();

  pidNhiet.setWindup(tWindupMin, tWindupMax, tKw);
  pidNhiet.setOutput(tOutMin, tOutMax);

  pidTocDoNhiet.setWindup(sWindupMin, sWindupMax, sKw);
  pidTocDoNhiet.setOutput(sOutMin, sOutMax);
}

void loop() {
  if (Serial.available()) {
    String s = Serial.getString();
    if (strcmp(s, "on") == 0) {
      run = true;
    } else if (strcmp(s, "off") == 0) {
      run = false;
    }
  }

  temp = LocCamBienBuong.updateEstimate(fLayNhietDoThucTe(ptBuong));
  if (millis() - t >= 5000 && run) {
    t = millis();
    //PID nhiệt độ
    err = 37.0f - temp;
    speed = pidNhiet.getPIDcompute(err);
    //PID tốc độ nhiệt
    currSpeed = temp - preTemp;
    thoiGianKichTriac = pidTocDoNhiet.getPIDcompute(speed - currSpeed);  //ms

    //cơ cấu
    Buong.turnOnPinAndDelayOff(thoiGianKichTriac);

    preTemp = temp;
  }
  Serial.printf("sp %f temp: %f speed %f currSpeed %f thoigian:%d\n", 37.0f, temp, speed, currSpeed, thoiGianKichTriac);
  thoiGianKichTriac = 0;
  delay(1000);
}

void khoiTaoCamBien() {
  pinMode(PT100_CS1_PIN, OUTPUT);
  pinMode(PT100_CS2_PIN, OUTPUT);
  pinMode(PT100_CS3_PIN, OUTPUT);
  digitalWrite(PT100_CS1_PIN, 1);
  digitalWrite(PT100_CS2_PIN, 1);
  digitalWrite(PT100_CS3_PIN, 1);
  ptBuong.begin(MAX31865_4WIRE);
  ptBuong.enable50Hz(true);

  for (uint8_t i = 0; i < 20; i++) {
    preTemp = LocCamBienBuong.updateEstimate(fLayNhietDoThucTe(ptBuong));
    Serial.printf("%f\n", preTemp);
  }
}
void khoiTaoQuat() {
  quat.init();
  quat.TurnOnTriac();
  quat.SetTimeOverFlow(4000);
}
void khoiTaoTriac() {
}

static float fLayNhietDoThucTe(Adafruit_MAX31865& xPt100) {
  float fNhietDo = 999;
  uint8_t u8Try = 5;
  while (u8Try--) {
    fNhietDo = xPt100.temperature(MAX31865_DIEN_TRO_CAM_BIEN, MAX31865_DIEN_TRO_THAM_CHIEU);
    if (abs(fNhietDo) < MAX31865_NHIET_DO_TOI_DA) {
      break;
    }
    fNhietDo = 999;
    xPt100.begin(MAX31865_4WIRE);
    xPt100.enable50Hz(true);
    delay(10);
  }
  return fNhietDo;
}
