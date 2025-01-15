#include "OnOffSoftTimer.h"

#define INIT_SUCC (1<<0)
#define TIMER_RUNNING (1<<1)
#define TIMER_REPEAT (1<<2)

#define SET_FLAG(flag, bit) ((flag) |= (bit)) 
#define RST_FLAG(flag, bit) ((flag) &= ~(bit))
#define CHECK_FLAG(flag, bit) ((flag) & (bit))
#define CHECK_INIT(flag) do {\
                            if(!CHECK_FLAG(flag, INIT_SUCC)){\
                              Serial.println("chưa khởi tạo hoặc khởi tạo thất bại");\
                              return;\
                            }\
                          } while (0)

#define TAG "ON OFF LIB"

void OnOffPin::init(BaseType_t time, void* arg, const char* nameTimer) {

    pinMode(m_i8Pin, OUTPUT);
    turnOffPin();
    this->m_pvArg = arg;

    // khởi tạo delay
    m_xTimerHandler = NULL;
    m_xTimerHandler = xTimerCreate(nameTimer, time, pdFALSE, (void*)this, timerCallBack);
    if (m_xTimerHandler == NULL) {
        Serial.println("Timer init fault\n");
        RST_FLAG(status, INIT_SUCC);
        return;
    }
    xTimerStop(this->m_xTimerHandler, portMAX_DELAY);
#ifdef QUEUE
    // khởi tạo hàng đợi 
    m_xQueueHandler = NULL;
    m_xQueueHandler = xQueueCreate(5, sizeof(TimeType_t));
    if (m_xTimerHandler == NULL) {
        Serial.println("Queue init fault\n");
        RST_FLAG(status, INIT_SUCC);
        return;
    }
#endif //QUEUE
    SET_FLAG(status, INIT_SUCC);
}
bool OnOffPin::isTimerRunning() {
    return CHECK_FLAG(status, TIMER_RUNNING);
}

void OnOffPin::setPinAndDelay(int8_t pinStatus, int16_t time, OnOffType_t type, const char* funcCall) {
    ESP_LOGI(TAG, "%s() call %s Time: %u", funcCall, __func__, time);
    CHECK_INIT(status);
    BaseType_t tick = pdMS_TO_TICKS(time);
    TimeType_t temDataTimer = {
        .m_u16Time = tick,
        .typeDelay = type
    };
    if (CHECK_FLAG(status, TIMER_RUNNING)) {
        Serial.printf("%s() timer is Running\n", __func__);
#ifdef QUEUE
        if (xQueueSend(m_xQueueHandler, &temDataTimer, 100) == pdFALSE) {
            Serial.printf("%s, queueFull\n", __func__);
        }
#endif //QUEUE
        return;
    }
    // thông báo timer đang chạy, các lệnh khác sẽ đưa vào queue nếu có 
    SET_FLAG(status, TIMER_RUNNING);
    dataTime = temDataTimer;

    ESP_LOGI(TAG, "%s() set timer oke", __func__);
    // trạng thái chân 0 hoặc 1 thì bật, nếu âm thì ko thực hiện 
    if (pinStatus >= 0) {
        writePin(pinStatus);
    }
    xTimerChangePeriod(this->m_xTimerHandler, tick, portMAX_DELAY);
}


void OnOffPin::turnOnPinAndDelayOff(uint16_t time, const char* funcCall) {
    ESP_LOGI(TAG, "%s() call %s Time: %u", funcCall, __func__, time);
    setPinAndDelay(1, time, eDELAY_OFF, __func__);
}
void OnOffPin::turnOffPinAndDelayOn(uint16_t time, const char* funcCall) {
    ESP_LOGI(TAG, "%s() call %s Time: %u", funcCall, __func__, time);
    setPinAndDelay(0, time, eDELAY_ON, __func__);
}
void OnOffPin::setDelayOff(uint16_t time, const char* funcCall) {
    ESP_LOGI(TAG, "%s() call %s Time: %u", funcCall, __func__, time);
    setPinAndDelay(-1, time, eDELAY_OFF, __func__);
}
void OnOffPin::setDelayOn(uint16_t time, const char* funcCall) {
    ESP_LOGI(TAG, "%s() call %s Time: %u", funcCall, __func__, time);
    setPinAndDelay(-1, time, eDELAY_ON, __func__);
}

/**
 *@brief tạo chu kỳ bật tắt, truyền giá trị thời gian nhỏ 100mS để thoát khỏi chế độ 
 *  
 * @param timeOn giá trị lớn hơn 100mS, thời gian giữ chân mức cao
 * @param timeOff giá trị lớn hơn 100mS, thời gian giữ chân mức thấp 
 * @param funcCall __func__, dùng để debug
 */
void OnOffPin::setRepeat(uint16_t timeOn, uint16_t timeOff, const char* funcCall) {
    ESP_LOGI(TAG, "%s() call %s TimeOn: %u, TimeOff: %u", funcCall, __func__, timeOn, timeOff);
    CHECK_INIT(status);
    if (timeOn < 100 || timeOff < 100) {
        RST_FLAG(status, TIMER_REPEAT);
        dataTimeOn.m_u16Time = dataTimeOff.m_u16Time = 0;
        return;
    }

    dataTimeOn.m_u16Time = timeOn;
    dataTimeOff.m_u16Time = timeOff;
    SET_FLAG(status, TIMER_REPEAT);

    turnOnPinAndDelayOff(dataTimeOn.m_u16Time, __func__);
}

void OnOffPin::timerCallBack(TimerHandle_t xTimer) {
    void* ptrID = pvTimerGetTimerID(xTimer);
    OnOffPin* pClass = (OnOffPin*)ptrID;

    // thông báo timer đã chạy xog delay lúc trước 
    RST_FLAG(pClass->status, TIMER_RUNNING);

    //thực hiện bật tắt theo yêu cầu trước 
    if (pClass->dataTime.typeDelay == eDELAY_ON) {
        pClass->turnOnPin();
        if (CHECK_FLAG(pClass->status, TIMER_REPEAT)) {
            pClass->setDelayOff(pClass->dataTimeOn.m_u16Time, __func__);
        }
    }
    else {
        pClass->turnOffPin();
        if (CHECK_FLAG(pClass->status, TIMER_REPEAT)) {
            pClass->setDelayOn(pClass->dataTimeOff.m_u16Time, __func__);
        }
    }
#ifdef QUEUE
    // kiểm tra trong queue có còn lệnh chờ nào không và thực hiện
    TimeType_t dataTime;
    if (xQueueReceive(pClass->m_xQueueHandler, &dataTime, 0)) {
        setPinAndDelay(-1, dataTime.m_u16Time, dataTime.typeDelay, __func__);
    }
#endif //QUEUE
}
