// #include "OnOff.h"
#include "OnOffHighResolutionTimer.h"


class a {
public:
  a(int p = 2)
    : pin(p) {
  }
  void init() {
    pin.init();
  }
  void setTime(uint64_t a) {
    pin.turnOnPinAndDelayOff(a);
  }
  bool isRun(){
    return pin.isTimerRunning();
  }
private:
  HRTOnOffPin pin;
};

a pin(2);

void task(void *ptr) {

  while (1) {
    for (int i = 0; i < 10; i++)
      // Serial.println("helooooo");
      delay(1);
  }
}

void setup() {
  Serial.begin(115200);
  pin.init();

  delay(1000);
  xTaskCreate(task, "task", 4048, NULL, 1, NULL);
  xTaskCreate(task, "task", 4048, NULL, 1, NULL);
  xTaskCreate(task, "task", 4048, NULL, 1, NULL);
  xTaskCreate(task, "task", 4048, NULL, 1, NULL);
  xTaskCreate(task, "task", 4048, NULL, 3, NULL);
  xTaskCreate(task, "task", 4048, NULL, 3, NULL);
  xTaskCreate(task, "task", 4048, NULL, 3, NULL);
  xTaskCreate(task, "task", 4048, NULL, 3, NULL);
}

void loop() {
  if (Serial.available()) {
    String t = Serial.readString();
    Serial.printf("%s\n", t);
    if (!pin.isRun()) {
      int a = t.toInt();
      Serial.printf("%d\n", a);
      pin.setTime(a);
    }
  }

  delay(1);
}
