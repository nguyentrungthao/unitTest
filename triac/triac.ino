#include "12_triac.h"

#define triacPin 4
#define acdetPin 34

triac t((gpio_num_t) triacPin, TIMER_GROUP_0, TIMER_0);

void setup() {
  // put your setup code here, to run once:
  t.init();
  triac::configACDETPIN((gpio_num_t)acdetPin);
  t.TurnOnTriac();
  t.SetTimeOverFlow(5000);
}

void loop() {
  // put your main code here, to run repeatedly:
}
