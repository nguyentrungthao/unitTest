#include "08_PID.h"

//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM//
//----------------Hàm khởi tạo đối tượng PID--------//
PID::PID() {
  this->Kp = 1;
  this->Ki = 0;
  this->Kd = 0;
  this->Sample_time = 100;
  this->WindupMax = 0;
  this->WindupMin = 0;
  this->OutMax = 0;
  this->OutMin = 0;
  this->ITerm = 0;
}
//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM//



//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM//
//----------------Hàm khoi tạo đối tương PID có truyền thông số---------//
PID::PID(float Kp, float Ki = 0, float Kd = 0, float Sample_time = 1000) {
  this->Kp = Kp;
  this->Ki = Ki;
  this->Kd = Kd;
  this->Sample_time = Sample_time;
  this->WindupMax = 0;
  this->WindupMin = 0;
  this->OutMax = 0;
  this->OutMin = 0;
  this->ITerm = 0;
  this->PTerm = 0;
  this->DTerm = 0;
}
//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM//


//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM//
//----------------Hàm thay đổi hệ số PID------------//
void PID::setPIDparamters(float Kp, float Ki = 0, float Kd = 0) {
  this->Kp = Kp;
  this->Ki = Ki;
  this->Kd = Kd;
}
//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM//


//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM//
//----------------Hàm tính toán PID-----------------//
float PID::getPIDcompute(float Error) {
  // PTERM
  this->PTerm = this->Kp * Error;

  // ITERM
  // Test Ki nhân ở ngõ ra
  this->ITerm += (this->Ki * Error + this->feedBackWindup * Kw) * (this->Sample_time / 1000);
  if (this->ITerm > this->WindupMax) this->ITerm = this->WindupMax;
  else if (this->ITerm < this->WindupMin) this->ITerm = this->WindupMin;
  //DTERM
  this->DTerm = ((Error - this->LastError) * this->Kd) / (this->Sample_time / 1000);
  this->LastError = Error;
  //OUTPUT
  this->Output = this->PTerm + this->ITerm + this->DTerm;
  if (this->Output > this->OutMax) {
    this->feedBackWindup = this->OutMax - this->Output;
    this->Output = this->OutMax;
  } else if (this->Output < this->OutMin) {
    this->feedBackWindup = this->OutMin - this->Output;
    this->Output = this->OutMin;
  } else {
    this->feedBackWindup = 0;
  }
  ESP_LOGI("PID", "P %f,I %f,D %f, Out %f", PTerm, ITerm, DTerm, Output);
  return this->Output;
}
//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM//



//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM//
//----------------Hàm set bão hòa khâu I------------//
void PID::setWindup(float MinValue, float MaxValue, float Kw) {
  this->WindupMax = MaxValue;
  this->WindupMin = MinValue;
  this->Kw = Kw;
}
//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM//


//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM//
//----------------Hàm set bão hòa OUTPUT------------//
void PID::setOutput(float MinValue, float MaxValue) {
  this->OutMax = MaxValue;
  this->OutMin = MinValue;
}
//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM//


//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM//
//----------------Hàm set thời gian lấy mẫu---------//
void PID::setSampleTime(float value) {
  this->Sample_time = value;
}
//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM//


float PID::getWindupMax() {
  return this->WindupMax;
}
float PID::getWindupMin() {
  return this->WindupMin;
}
float PID::getSampleTime() {
  return this->Sample_time;
}
float PID::getKp() {
  return this->Kp;
}
float PID::getKi() {
  return this->Ki;
}
float PID::getKd() {
  return this->Kd;
}
float PID::getOutputMin() {
  return this->OutMin;
}
float PID::getOutputMax() {
  return this->OutMax;
}
