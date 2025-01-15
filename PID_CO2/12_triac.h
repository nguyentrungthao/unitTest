

#ifndef _TRIAC_H_
#define _TRIAC_H_

#ifdef __cplusplus 
extern "C"{
#endif

#include "driver/gpio.h"
#include "driver/timer.h"


#define TRIAC_LOW_LIMIT  1900
#define TRIAC_HIGH_LIMIT 9600
#define TRIAC_HOLD_TIME  100


class triac{
    public:
        triac(gpio_num_t pin = GPIO_NUM_1, timer_group_t grp = TIMER_GROUP_0, timer_idx_t idx = TIMER_0);

        static void configACDETPIN(gpio_num_t acdet_pin = GPIO_NUM_1);
        void init(void);
        void SetTimeOverFlow(uint16_t timeOverFlow = 9600);
        void TurnOnTriac();
        void TurnOffTriac();
        bool CheckAcdet();

        static gpio_num_t acdet;
        gpio_num_t pin = GPIO_NUM_1;

        timer_group_t grp = TIMER_GROUP_0;
        timer_idx_t   idx = TIMER_0;

        bool dis_timer = false; 
        uint16_t timeOverFlow = TRIAC_HIGH_LIMIT;
        bool RunStatus = false;
    private:
};    

#ifdef __cplusplus 
}
#endif

#endif 