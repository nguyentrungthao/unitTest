#ifndef _HEATER_H_
#define _HEATER_H_

#define BAO

#include "Config.h"
#include "SimpleKalmanFilter.h"  // thư viện lọc dữ liệu
#include "Adafruit_MAX31865.h"
#include "cmath"

#include "12_triac.h"
#include "OnOffHighResolutionTimer.h"
#include "driver/gpio.h"
#include "driver/timer.h"
#include "hal/gpio_types.h"
#include "hal/timer_types.h"
#include "AnhLABV01HardWare.h"

// #define DEBUG

#define TRIAC_BUONG_PIN TRIAC1_PIN
#define TRIAC_QUAT_PIN TRIAC2_PIN
#define TRIAC_CUA_PIN TRIAC3_PIN
#define TRIAC_SAU_PIN TRIAC4_PIN

#define MAX_CS_BUONG_PIN PT100_CS1_PIN  // cảm biến PT100 số 1 là cảm biến trong buồng
#define MAX_CS_VACH_PIN PT100_CS3_PIN   // cảm biến PT100 vách là cảm biến số 3 trên board
#define MAX_CS_CUA_PIN PT100_CS2_PIN    // cảm biến  PT100 nhiệt trên cửa là cảm biến số 2
#define MAX_MOSI_PIN PT100_MOSI_PIN      // ESP32 OUT ADS IN
#define MAX_MISO_PIN PT100_MISO_PIN      // ESP32 IN ADS OUT
#define MAX_CLK_PIN PT100_SCK_PIN        // ESP32 CLK ADS CLK

#define _BAT_DIEU_KHIEN_NHIET_DO true
#define _TAT_DIEU_KHIEN_NHIET_DO false

#define HIEU_SUAT_TRUYEN_NHIET_BUONG 0.8f  // hiệu suất truyền nhiệt kim loại
#define HIEU_SUAT_TRUYEN_NHIET_KHI 0.8f    // hiệu suất truyền nhiệt không khí

#define HIEU_SUAT_CHENH_LECH 1.3f  // hệ số chênh lệch giữa gia nhiệt ở xa setpoint và gần setpoint (khoảng cách 0.2 độ)

#define KHOI_LUONG_BUONG 17.0f                          // kg
#define THE_TICH_BUONG 55.0f                            // lít
#define KHOI_LUONG_KHI ((THE_TICH_BUONG)*1.28f * 1e-3)  // V * p * 1^-3 = kg
#define NHIET_DUNG_RIENG_BUONG 500.0f
#define NHIET_DUNG_RIENG_KHI 1005.0f
#define GIA_TRI_BAT_TRIAC 3500U  // tương ứng 202V khi đầu vào là 220V

#define HIEU_SUAT_TRUYEN_NHIET_CUA 0.8f
#define HIEU_SUAT_TRUYEN_NHIET_KINH 0.8f
#define KHOI_LUONG_CUA 5.0f
#define NHIET_DUNG_RIENG_CUA NHIET_DUNG_RIENG_BUONG
#define KHOI_LUONG_KINH 2.5f          // KG
#define NHIET_DUNG_RIENG_KINH 800.0f  // j/kg

#define KHOI_LUONG_VANH 2.0f
#define NHIET_DUNG_RIENG_VANH NHIET_DUNG_RIENG_BUONG
#define HIEU_SUAT_TRUYEN_NHIET_VANH 0.8f
#define TONG_TRO_VANH 300.0f

#define DIEN_AP_VAO_HIEU_DUNG 220.0f  // điện áp
// #define POWER 1950.0f
// #define TONG_TRO_BUONG_DOT((DIEN_AP_VAO_HIEU_DUNG * DIEN_AP_VAO_HIEU_DUNG) / (POWER))
#define TONG_TRO_BUONG 66.8
#define TONG_TRO_CUA 100
#define PI 3.14159265358979323846f

#define HeatingTime 30
#define CoolingTime 30
// #define HeatingPower  6000
#define GIA_TRI_BAT_TRIAC_MAX 2000
#define GIA_TRI_BAT_TRIAC_MIN 9600

#define HeatingFlag true
#define CoolingFlag false

//* thông số cho đọc nhiệt độ
#define MAX31865_NHIET_DO_TOI_DA 200.0f
#define MAX31865_DIEN_TRO_THAM_CHIEU 390.0
#define MAX31865_DIEN_TRO_CAM_BIEN 100.0
//* thông số cho đọc nhiệt độ

//* thông số cho bộ lọc Kalman
#define SAI_SO_DO 1
#define SAI_SO_UOC_LUONG 1
#define HE_SO_NHIEU 2
//* thông số cho bộ lọc Kalman

#if defined(ON_OFF_DEBUG) && defined(debug) && defined(DEBUG_HEATER)
#define NHIET_SerialPrintf(flag, string, ...) \
  do { \
    if (flag) \
      Serial.printf(string, ##__VA_ARGS__); \
  } while (0)

#define NHIET_SerialPrintln(flag, string, ...) \
  do { \
    if (flag) \
      Serial.println(string, ##__VA_ARGS__); \
  } while (0)
#define NHIET_SerialPrint(flag, string, ...) \
  do { \
    if (flag) \
      Serial.print(string, ##__VA_ARGS__); \
  } while (0)

#elif !defined(ON_OFF_DEBUG) && defined(debug) && defined(DEBUG_HEATER)

#define NHIET_SerialPrintf(flag, string, ...) Serial.printf(string, ##__VA_ARGS__)
#define NHIET_SerialPrintln(flag, string, ...) Serial.println(string, ##__VA_ARGS__)
#define NHIET_SerialPrint(flag, string, ...) Serial.print(string, ##__VA_ARGS__)

#else

#define NHIET_SerialPrintf(flag, string, ...) (void*)0
#define NHIET_SerialPrintln(flag, string, ...) (void*)0
#define NHIET_SerialPrint(flag, string, ...) (void*)0

#endif

typedef enum {
  eTinhToanGiaTriDieuKhien,
  GiaNhiet,
  LamLanh,
} TrangThaiDieuKhienLogic;

typedef enum {
  NhietGiamCham,
  NhietGiamNhanh,
  NhietTangNhanh,
  NhietTangCham,
  KhongGiaNhiet,
  KhongXacDinh,
} TrangThaiNhiet_t;

class HEATER : public triac {
public:
  float fSetpointNhiet;            // Giá trị setpoint để điều khiển nhiệt đạt tới.
  float fSetpointChoCua;           // Giá trị setpoint nhiệt độ cho cửa
  float fSetpointChoVanh;          // Giá trị setpoint nhiệt độ cho cửa
  float SaiSoNhietChoPhep = 0.25;  // Nếu nhiệt độ thực tế trong giới hạn +/- 0.25 thì cho giá trị nhiệt thực tế = setpoint luôn.
  float HeSoCalib = 0;             // HeSoCalib = NhietDoChuan - (NhietDoHienThi - HeSoCalib)
  bool CoChoPhepLogDebug = true;   // cờ cho phép log serial0 để debug
  uint8_t demOnDinh;               // đếm số lần nhiệt vào sai số -+0.25
  uint32_t fanPower;               // góc kích triac quy định điện áp cung cấp cho quạt
  uint32_t ThoiGianBatQuat, ThoiGianTatQuat;
  uint32_t offset;

  HEATER(void);
  void KhoiTao();

  void CaiDatNhietDo(float fSetpointNhiet = -1);

  void TatDieuKhienNhietDo(bool RESET = true);
  void BatDieuKhienNhietDo(bool RESET = true);

  float LayNhietDoLoc() {
    return nhietDoLoc;
  }
  float LayNhietDoLoc_CUA() {
    return NhietDoLoc_CUA;
  }
  float LayNhietDoLoc_VANH() {
    return NhietDoLoc_VANH;
  }
  void DelayTinhToanGiaTriDieuKhien();
  void logDEBUG();

private:
  bool trangThaiDieuKhien;  // trạng thái điều khiển của file heater.cpp

  Adafruit_MAX31865 PT100_buong = Adafruit_MAX31865(MAX_CS_BUONG_PIN, MAX_MOSI_PIN, MAX_MISO_PIN, MAX_CLK_PIN);
  Adafruit_MAX31865 PT100_vanh = Adafruit_MAX31865(MAX_CS_VACH_PIN, MAX_MOSI_PIN, MAX_MISO_PIN, MAX_CLK_PIN);
  Adafruit_MAX31865 PT100_cua = Adafruit_MAX31865(MAX_CS_CUA_PIN, MAX_MOSI_PIN, MAX_MISO_PIN, MAX_CLK_PIN);
  SimpleKalmanFilter LocCamBienBuong = SimpleKalmanFilter(SAI_SO_DO, SAI_SO_UOC_LUONG, HE_SO_NHIEU);
  SimpleKalmanFilter LocCamBienVanh = SimpleKalmanFilter(SAI_SO_DO, SAI_SO_UOC_LUONG, HE_SO_NHIEU);
  SimpleKalmanFilter LocCamBienCua = SimpleKalmanFilter(SAI_SO_DO, SAI_SO_UOC_LUONG, HE_SO_NHIEU);

  HRTOnOffPin triacBuong;
  HRTOnOffPin triacSau;
  HRTOnOffPin triacCua;
  triac triacQuat;  // triac điều khiển tốc độ quạt

  float NhietDoThucTe;  // nhiệt thực tế trong buồng
  float nhietDoLoc;     // nhiệt độ lọc trong buông

  float NhietDoLoc_CUA;
  float NhietDoLoc_VANH;

  //các biến chạy cho mặt sau của buồng
  
  // các biến trong hàm ChayQuyTrinhGiaNhiet buồng
  TrangThaiNhiet_t TempState;
  TrangThaiDieuKhienLogic step;
  int32_t HeatUpTime, CoolDownTime;
  uint32_t HeatUpTimeMatSau;    // thời gian nhiệt chạy
  int32_t giaTriGiaKhauI = 0;  // giá trị giả khâu I trong PID để bù phần nhiệt còn thiếu
  int32_t u16ThoiGianGiaNhiet;    // thời gian nhiệt chạy


  // các biến trong hàm ChayQuyTrinhGiaNhiet cửa
  int32_t HeatUpTime_CUA, CoolDownTime_CUA;
  int32_t u16ThoiGianGiaNhiet_CUA;

  //các biến trong hàm giám sát nhiệt độ cửa
  TrangThaiDieuKhienLogic step_CUA;
  TrangThaiNhiet_t TempState_CUA;

  //các biến trong hàm giám sát nhiệt độ VANH
  TrangThaiDieuKhienLogic step_VANH;
  TrangThaiNhiet_t TempState_VANH;

  void GiaiThuat();  // giải thuật nhiệt trong buồng

  void ChayQuyTrinhGiaNhiet();      // chu trình nhiệt trong buồng
  void ChayQuyTrinhGiaNhiet_CUA();  //* chu trình nhiệt cửa
  void ChayQuyTrinhGiaNhiet_VANH();

  void GiamSatNhietDo();      // giám sát nhiệt độ trong buồng
  void GiamSatNhietDo_CUA();  //* giám sát nhiệt độ cửa
  void GiamSatNhietDo_VANH();
  void DieuKhienQuat();

  static void TASK_TinhToanGiaTriDieuKhien(void* _this);

  TaskHandle_t taskTriacHandle;
  TaskHandle_t taskCalculateHandle;
  esp_timer_handle_t tTriacHandle;
  TimerHandle_t TimerLayMauNhietDo;

  bool DoorTriacOnFlag = false;
};


#endif