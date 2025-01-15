#include "12_triac.h"
#include "08_PID.h"
#include "OnOffHighResolutionTimer.h"
#include "AnhLABV01HardWare.h"
#include <Adafruit_MAX31865.h>
#include "SimpleKalmanFilter.h"  // thư viện lọc dữ liệu

// Use software SPI: CS, DI, DO, CLK
// Adafruit_MAX31865 ptBuong = Adafruit_MAX31865(PT100_CS1_PIN, PT100_MOSI_PIN, PT100_MISO_PIN, PT100_SCK_PIN);
Adafruit_MAX31865 ptBuong = Adafruit_MAX31865(PT100_CS1_PIN, PT100_MOSI_PIN, PT100_MISO_PIN, PT100_SCK_PIN);
Adafruit_MAX31865 ptCua = Adafruit_MAX31865(PT100_CS2_PIN, PT100_MOSI_PIN, PT100_MISO_PIN, PT100_SCK_PIN);
Adafruit_MAX31865 ptDu = Adafruit_MAX31865(PT100_CS3_PIN, PT100_MOSI_PIN, PT100_MISO_PIN, PT100_SCK_PIN);
SimpleKalmanFilter LocCamBienBuong = SimpleKalmanFilter(1, 1, 2);

#define Kp 50
#define Ki 5
#define Kd 0
#define Kw 7
#define OutMax 5000  //ms
#define OutMin 0
#define WindupMax 5000
#define WindupMin 0

#define MAX31865_NHIET_DO_TOI_DA 200.0f
#define MAX31865_DIEN_TRO_THAM_CHIEU 390.0
#define MAX31865_DIEN_TRO_CAM_BIEN 100.0

triac quat((gpio_num_t)TRIAC4_PIN, TIMER_GROUP_0, TIMER_0);
PID pidBuong(Kp, Ki, Kd, 5000);
HRTOnOffPin Buong(TRIAC1_PIN);

typedef struct saveData {
  float spBuong;
  float spCua;
} data;

int t = 0;
float err;
int thoiGianKichTriac;
float temp, preTemp;

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

  pidBuong.setWindup(WindupMin, WindupMax, Kw);
  pidBuong.setOutput(OutMin, OutMax);
}

void loop() {
  temp = LocCamBienBuong.updateEstimate(fLayNhietDoThucTe(ptBuong));
  err = 37.0f - temp;
  if (millis() - t >= 5000) {
    thoiGianKichTriac = pidBuong.getPIDcompute(err);  //ms
    Buong.turnOnPinAndDelayOff(thoiGianKichTriac);
    Serial.printf("thoigian: %d, tốc độ %0.2f,\t", thoiGianKichTriac, temp - preTemp);
    preTemp = temp;
    t = millis();
  }
  Serial.printf("sp %0.2f, t: %0.2f\n", 37.0f, temp );
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

  for(uint8_t i = 0 ; i < 255; i++){
    preTemp = LocCamBienBuong.updateEstimate(fLayNhietDoThucTe(ptBuong)); 
  }
}
void khoiTaoQuat() {
  quat.init();
  quat.TurnOnTriac();
  quat.SetTimeOverFlow(5000);
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
