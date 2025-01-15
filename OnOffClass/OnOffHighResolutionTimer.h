#ifndef _ON_OFF_HIGH_RESOLUTION_H_
#define _ON_OFF_HIGH_RESOLUTION_H_

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_timer.h"

#define INFINITY -1
#define REPEAT_ONE 0

//define queue để lưu các event đã ra lệnh trên chân
// #define QUEUE

typedef enum {
  eDELAY_ON,
  eDELAY_OFF,
} OnOffType_t;

class HRTOnOffPin {
public:
  HRTOnOffPin(int8_t pin)
    : m_i8Pin(pin) {
    dataTimeOn.m_u64Time = 0;
    dataTimeOn.typeDelay = eDELAY_OFF;

    dataTimeOff.m_u64Time = 0;
    dataTimeOff.typeDelay = eDELAY_ON;

    dataTime.m_u64Time = 0;
  }
  void init(BaseType_t time = 1000, void* arg = NULL, const char* nameTimer = "");

  void init(void*(func)(void*), void* arg) {
    func(arg);
  }

  // nên check trạng thái chạy của timer trước khi gọi
  bool isTimerRunning();
  /*
      khi gọi các hàm SetDelay, timer sẽ được chạy nếu đang rảnh
      thực hiện callBack khi hết thời gian
    */
  // bật chân mức 1 và tắt sau 1 khoảng thời gian
  void turnOnPinAndDelayOff(uint64_t time, const char* funcCall = "");
  // tắt chân và bật chân sau 1 khoảng thời gian
  void turnOffPinAndDelayOn(uint64_t time, const char* funcCall = "");
  //chỉ cài thời gian tắt chân
  void setDelayOff(uint64_t time, const char* funcCall = "");
  // chỉ cài thời gian bật chân
  void setDelayOn(uint64_t time, const char* funcCall = "");
  // bật trạng thái chân và lựa chọn tự kiện bật tắt
  void setPinAndDelay(int8_t pinStatus, uint64_t time, OnOffType_t type, const char* funcCall = "");

  // chỉ bật chân
  void turnOnPin() {
    writePin(1);
  }
  void turnOnPin(void*(func)(void*), void* arg) {
    func(arg);
  }
  // chỉ tắt chân
  void turnOffPin() {
    writePin(0);
  }
  void turnOffPin(void*(func)(void*), void* arg) {
    func(arg);
  }
  void writePin(int8_t pinStatus) {
    digitalWrite(m_i8Pin, pinStatus);
  }
  void writePin(void*(func)(void*), void* arg) {
    func(arg);
  }

  // trả về trạng thái chân
  bool getStatusPin() {
    return digitalRead(m_i8Pin);
  }

  bool turnOnPin(bool*(func)(void*), void* arg) {
    return func(arg);
  }
private:
  typedef struct {
    uint64_t m_u64Time;  // thời gian delay
    uint8_t typeDelay;   // delay on hay delay off
  } TimeType_t;

  TimeType_t dataTime;
  int8_t m_i8Pin;
  void* m_pvArg;

  // dùng cho repeat
  TimeType_t dataTimeOn;
  TimeType_t dataTimeOff;

  esp_timer_handle_t m_xTimerHandler;
  uint8_t status;
#ifdef QUEUE
  QueueHandle_t m_xQueueHandler;
#endif  //QUEUE
  static void timerCallBack(void* arg);
};


#endif  //_ON_OFF_HIGH_RESOLUTION_H_