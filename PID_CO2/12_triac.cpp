#include "12_triac.h"
#include <Arduino.h>

gpio_num_t triac::acdet = GPIO_NUM_1;
static triac *triac_set[5];
static uint8_t num_triac = 0;
static uint8_t countAcdet = 0;
static void IRAM_ATTR acdet_intr_handler(void *arg);
static bool IRAM_ATTR timer_triac_intr_handler(void *arg);

triac::triac(gpio_num_t pin, timer_group_t grp, timer_idx_t idx){
    this->pin = pin;
    this->grp = grp;
    this->idx = idx;
    this->RunStatus = false;
}


void triac::configACDETPIN(gpio_num_t acdet_pin) {
    triac::acdet = acdet_pin;

    gpio_config_t acdet_pin_conf = {
        .pin_bit_mask = (uint64_t)(1ULL << acdet_pin),
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_NEGEDGE,
    };
    gpio_config(&acdet_pin_conf);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(triac::acdet, acdet_intr_handler, NULL);
    gpio_intr_enable(triac::acdet);
}


void triac::init(void){
    pinMode(this->pin, OUTPUT);

    timer_config_t triac1_tim_conf = {
        .alarm_en    = TIMER_ALARM_EN,
        .counter_en  = TIMER_PAUSE,
        .counter_dir = TIMER_COUNT_UP,
        .auto_reload = TIMER_AUTORELOAD_EN,
        .divider     = 80,
    }; 
    timer_init             (this->grp, this->idx, &triac1_tim_conf);
    timer_set_counter_value(this->grp, this->idx, 0);
    timer_isr_callback_add (this->grp, this->idx, timer_triac_intr_handler, (void *)this, NULL);
    timer_enable_intr      (this->grp, this->idx);

    triac_set[num_triac++] = this;
    delayMicroseconds(1000);
    digitalWrite(this->pin, 0);
}


void triac::SetTimeOverFlow(uint16_t timeOverFlow) {
    // gpio_intr_disable(triac::acdet);
    // timer_pause(this->grp, this->idx);
    // timer_disable_intr(this->grp, this->idx);
    // timer_set_counter_value(this->grp, this->idx, 0);
    if (timeOverFlow > TRIAC_HIGH_LIMIT) timeOverFlow = TRIAC_HIGH_LIMIT;
    if (timeOverFlow < TRIAC_LOW_LIMIT)  timeOverFlow = TRIAC_LOW_LIMIT;
    this->timeOverFlow = timeOverFlow;
//     timer_enable_intr(this->grp, this->idx);
//     gpio_intr_enable(triac::acdet);
}


// void triac::TurnOnTriac() {
//     this->RunStatus = true;
//     gpio_intr_enable(triac::acdet);
//     timer_enable_intr(this->grp, this->idx);
//     Serial.println("------ Bat triac -------");
// }
// void triac::TurnOffTriac() {
//     this->RunStatus = false;
//     // gpio_intr_disable(triac::acdet);
//     timer_disable_intr(this->grp, this->idx);
//     timer_set_counter_value(this->grp, this->idx, 0);
//     Serial.println("------ Tat triac -------");
// }

void triac::TurnOnTriac() {
    if(this->RunStatus == false) {
        this->RunStatus = true;
        timer_start(this->grp, this->idx);
    }
}
void triac::TurnOffTriac() {
    if(this->RunStatus == true) 
    {
        this->RunStatus = false;
        timer_pause(this->grp, this->idx);
        timer_set_counter_value(this->grp, this->idx, 0);
    }
}

static void IRAM_ATTR acdet_intr_handler(void *arg){
    if(digitalRead(triac::acdet)) return;
    countAcdet++;
    for(uint8_t i=0; i<num_triac; i++){
        triac *ptriac = triac_set[i];
        ptriac->dis_timer = false;
        digitalWrite(ptriac->pin, 0);
        if (ptriac->RunStatus == true && ptriac->timeOverFlow < TRIAC_HIGH_LIMIT) {
            timer_group_set_alarm_value_in_isr(ptriac->grp, ptriac->idx, ptriac->timeOverFlow);
            timer_start(ptriac->grp, ptriac->idx);
        }
    }
}


static bool IRAM_ATTR timer_triac_intr_handler(void *arg){
    triac *ptriac = (triac *)arg;

    if(ptriac->dis_timer == false) {
        digitalWrite(ptriac->pin, 1);
        timer_group_set_alarm_value_in_isr(ptriac->grp, ptriac->idx, TRIAC_HOLD_TIME);
        ptriac->dis_timer = true;

        return false;
    }
    else{
        digitalWrite(ptriac->pin, 0);
        timer_pause(ptriac->grp, ptriac->idx);
    }

    return false;
}

bool triac::CheckAcdet()
{
    if(countAcdet) {
        countAcdet = 0;
        return true;
    } 
    else {
        countAcdet = 0;
        return false;
    }
}
