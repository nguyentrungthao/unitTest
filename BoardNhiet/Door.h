
#ifndef _DOOR_H_
#define _DOOR_H_
#include <Arduino.h>
#include "Config.h"

#define DOOR_OPEN 1
#define DOOR_CLOSE 0

class DOOR {
public:
  void KhoiTao(void);
  bool TrangThai(void);
  uint8_t CoDongMoCua = DOOR_OPEN;
};
#endif
