#ifndef ESPNOW_2WAYS_H
#define ESPNOW_2WAYS_H

#include <esp_now.h>
#include <esp_wifi.h>
#include <string>
#include <WiFi.h>

#include "Config.h"

class ESPNOW {
public:
  typedef struct NhietDo_message {
    int TrangThaiONOFF;
    bool CoTrangThaiMay = false;  // biến này cho module heater biến là người dùng vừa ON máy bằng hmi hoặc APP ( kiểm tra trong chương trình chính cũ dòng 219)

    // Nhiệt
    float fSetpointNhiet;   // SP nhiệt tủ
    float fSetpointChoCua;  // SP nhiệt cửa
    float offset; // chênh lệch thời gian kích giữa lưng và buồng
    float NhietDo;          // Hay nhiệt độ hiện tại
    float NhietDo_CUA;
    float SaiSoNhietChoPhep;  // Nếu nhiệt độ thực tế trong giới hạn +/- 0.25 thì cho giá trị nhiệt thực tế = setpoint luôn.
    float HeSoCalib;
    float HeSoCalibNhietDo;
    uint16_t TrangThaiNhiet = 1;

    // Quạt
    bool BamNutQUAT = false;
    uint16_t fanPower = 9600;
    uint16_t LayThoiGianKichTriac = 9600;
    int TocDoQuat = 0;
    int ThoiGianBatQuat;  //Mới thêm vào
    int ThoiGianTatQuat;
    uint16_t TrangThaiQuat = 1;

    // Cửa
    uint16_t TrangThaiCua = 1;
    uint8_t CoDongMoCua;

    //Thời gian cài đặt
    int ThoiGianONCaiDat;  // Thời gian ON (là tgian cho phép chạy) cài đặt cho máy.

    float fSetpointChoVanh;
    float NhietDo_VANH;


  } DuLieuNhietDoTruyenNhan_t;
  static DuLieuNhietDoTruyenNhan_t DuLieuNhanDuocQuaESPNow;

  static String WIFI_SSID;
  int32_t channel;

  static bool FlagDeNhanDataKhoiTaoBanDau;

  bool KhoiTaoESPNow();
  void setupWiFiChannel(String _ssid);
  void GuiDuLieuQuaESPNow(const DuLieuNhietDoTruyenNhan_t& myData);

private:
  static void OnDataSent(const uint8_t* mac_addr, esp_now_send_status_t status);
  static void OnDataRecv(const uint8_t* mac, const uint8_t* incomingData, int len);
};

extern uint8_t broadcastAddress[6];

#endif