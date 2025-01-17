#include "Door.h"

void DOOR::KhoiTao(void) {
  pinMode(PIN_DOOR, INPUT);
  CoDongMoCua = TrangThai();
}

// Tiếp điểm của cửa thường mở, chân đọc đc kéo mức cao (1), khi đóng cửa thì mức thấp (0)
bool DOOR::TrangThai(void) {
  return digitalRead(PIN_DOOR) ? DOOR_CLOSE : DOOR_OPEN;  // Return trạng thái cửa
}