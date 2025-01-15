#include "OnOffHighResolutionTimer.h"

#define INIT_SUCC (1 << 0)
#define TIMER_RUNNING (1 << 1)
#define TIMER_REPEAT (1 << 2)

#define SET_FLAG(flag, bit) ((flag) |= (bit))
#define RST_FLAG(flag, bit) ((flag) &= ~(bit))
#define CHECK_FLAG(flag, bit) ((flag) & (bit))
#define CHECK_INIT(flag) \
  do { \
    if (!CHECK_FLAG(flag, INIT_SUCC)) { \
      Serial.println("chưa khởi tạo hoặc khởi tạo thất bại"); \
      return; \
    } \
  } while (0)

#define TAG "ON OFF LIB"

void HRTOnOffPin::init(BaseType_t time, void* arg, const char* nameTimer) {

  pinMode(m_i8Pin, OUTPUT);
  turnOffPin();
  this->m_pvArg = arg;

  esp_timer_create_args_t oneshot_timer_args = {
    .callback = timerCallBack,
    .arg = (void*)this,
    .dispatch_method = ESP_TIMER_TASK,
    .name = nameTimer
  };
  if (esp_timer_create(&oneshot_timer_args, &m_xTimerHandler) != ESP_OK) {
    Serial.println("Timer not create");
    RST_FLAG(status, INIT_SUCC);
    return;
  }
  RST_FLAG(status, TIMER_RUNNING);
  esp_timer_stop(m_xTimerHandler);

#ifdef QUEUE
  // khởi tạo hàng đợi
  m_xQueueHandler = NULL;
  m_xQueueHandler = xQueueCreate(5, sizeof(TimeType_t));
  if (m_xTimerHandler == NULL) {
    Serial.println("Queue init fault\n");
    RST_FLAG(status, INIT_SUCC);
    return;
  }
#endif  //QUEUE
  SET_FLAG(status, INIT_SUCC);
}
bool HRTOnOffPin::isTimerRunning() {
  return CHECK_FLAG(status, TIMER_RUNNING);
}

void HRTOnOffPin::setPinAndDelay(int8_t pinStatus, uint64_t time, OnOffType_t type, const char* funcCall) {
  uint64_t tick = pdMS_TO_TICKS(time);
  TimeType_t temDataTimer = {
    .m_u64Time = time,
    .typeDelay = type
  };
  if (CHECK_FLAG(status, TIMER_RUNNING)) {
    Serial.printf("%s() timer is Running\n", __func__);
#ifdef QUEUE
    if (xQueueSend(m_xQueueHandler, &temDataTimer, 100) == pdFALSE) {
      Serial.printf("%s, queueFull\n", __func__);
    }
#endif  //QUEUE
    return;
  }
  // thông báo timer đang chạy, các lệnh khác sẽ đưa vào queue nếu có
  SET_FLAG(status, TIMER_RUNNING);
  dataTime = temDataTimer;

  // trạng thái chân 0 hoặc 1 thì bật, nếu âm thì ko thực hiện
  if (pinStatus >= 0) {
    writePin(pinStatus);
  }
  esp_timer_stop(m_xTimerHandler);
  // chuyển sang microSecond
  esp_timer_start_once(m_xTimerHandler, dataTime.m_u64Time * 1000);
  ESP_LOGI(TAG, "%s() call %s Time: %u", funcCall, __func__, time);
}


void HRTOnOffPin::turnOnPinAndDelayOff(uint64_t time, const char* funcCall) {
  setPinAndDelay(1, time, eDELAY_OFF, funcCall);
}
void HRTOnOffPin::turnOffPinAndDelayOn(uint64_t time, const char* funcCall) {
  setPinAndDelay(0, time, eDELAY_ON, funcCall);
}
void HRTOnOffPin::setDelayOff(uint64_t time, const char* funcCall) {
  setPinAndDelay(-1, time, eDELAY_OFF, funcCall);
}
void HRTOnOffPin::setDelayOn(uint64_t time, const char* funcCall) {
  setPinAndDelay(-1, time, eDELAY_ON, funcCall);
}

void HRTOnOffPin::timerCallBack(void* arg) {
  HRTOnOffPin* pClass = (HRTOnOffPin*)arg;

  // thông báo timer đã chạy xog delay lúc trước
  RST_FLAG(pClass->status, TIMER_RUNNING);

  //thực hiện bật tắt theo yêu cầu trước
  if (pClass->dataTime.typeDelay == eDELAY_ON) {
    pClass->turnOnPin();
  } else {
    pClass->turnOffPin();
  }
#ifdef QUEUE
  // kiểm tra trong queue có còn lệnh chờ nào không và thực hiện
  TimeType_t dataTime;
  if (xQueueReceive(pClass->m_xQueueHandler, &dataTime, 0)) {
    setPinAndDelay(-1, dataTime.m_u64Time, dataTime.typeDelay, __func__);
  }
#endif  //QUEUE
}
