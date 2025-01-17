#include "ESPNOW2Ways.h"

// #ifndef TEST_CONG_THUC
// uint8_t broadcastAddress[6] = { 0x70, 0xb8, 0xf6, 0x96, 0x7C, 0x20 };  // 0x94, 0xe6, 0x86, 0xc5, 0x2a, 0x9c
// #else 
uint8_t broadcastAddress[6] = {};
// #endif 
esp_now_peer_info_t peerInfo;

bool ESPNOW::FlagDeNhanDataKhoiTaoBanDau = false;

ESPNOW::DuLieuNhietDoTruyenNhan_t ESPNOW::DuLieuNhanDuocQuaESPNow;
String ESPNOW::WIFI_SSID;

int32_t getWiFiChannel(const char *ssid) {

  return 0;
}

void ESPNOW::setupWiFiChannel(String _ssid) {
  ESPNOW::WIFI_SSID = _ssid;
}

void ESPNOW::OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  
}

void ESPNOW::OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {


}

bool ESPNOW::KhoiTaoESPNow() {
  
  return true;
}

void ESPNOW::GuiDuLieuQuaESPNow(const DuLieuNhietDoTruyenNhan_t &myData) {
 
}
