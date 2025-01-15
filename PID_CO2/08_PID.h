#ifndef _PID_h
#define _PID_h

#include "esp_check.h"
#include "Arduino.h"

class PID {
public:
  float Kp, Ki, Kd, Output, Kw;
  float Sample_time;
  float WindupMax, WindupMin;
  float OutMax, OutMin;
  float PTerm, ITerm, DTerm;
  float LastError;
  float feedBackWindup;
public:
  PID();
  PID(float, float, float, float);
  void setPIDparamters(float, float, float);
  void setWindup(float, float, float);
  void setOutput(float, float);
  void setSampleTime(float);  //ms

  float getPIDcompute(float);
  float getWindupMax();
  float getWindupMin();
  float getSampleTime();
  float getKp();
  float getKi();
  float getKd();
  float getOutputMin();
  float getOutputMax();
};

#endif
