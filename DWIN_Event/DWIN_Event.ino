
/*
 * -----------------------------------------------------------------------------------------
 * DWIN Hello World Sketch   | Author : Tejeet ( tejdwin@gmail.com )
 * -----------------------------------------------------------------------------------------
 * This is DWIN HMI Libabry for Arduino Compatible Boards. More Info about Display Visit
 * Officail Site --> https://www.dwin-global.com/
 * 
 * Example sketch/program showing how to initialize DWIN Hmi with Arduino Or ESP32 Boards
 * In this example we can see on setup loop we change the page no and set the brighness &
 * Listen to display Events from serial port
 * 
 * DWIN HMI to Various Boards Pinout Connection
 * -----------------------------------------------------------------------------------------
 * DWIN            ESP32         Arduino       ArduinoMega       ESP8266        
 * Pin             Pin           Pin           Pin               Pin  ( Coming Soon ) 
 * -----------------------------------------------------------------------------------------
 * 5V              Vin           5V            5V
 * GND             GND           GND           GND
 * RX2             16            2             18
 * TX2             17            3             19
 *------------------------------------------------------------------------------------------
 *
 * For More information Please Visit : https://github.com/dwinhmi/DWIN_DGUS_HMI
 *
 */


#include <Arduino.h>
#include <DWIN.h>

#define ADDRESS_A "1010"
#define ADDRESS_B "1020"
#define _VPAddressNhietDoThucTe 0x4100  // Địa chỉ VP hiển thị nhiệt độ thực tế
#define DGUS_BAUD 115200

// If Using ESP 32
#if defined(ESP32)
#define DGUS_SERIAL Serial2
DWIN hmi(DGUS_SERIAL, 16, 17, DGUS_BAUD);

// If Using Arduino Uno
#else
DWIN hmi(2, 3, DGUS_BAUD);
#endif

#define LED_BUILTIN 2

// Event Occurs when response comes from HMI
void onHMIEvent(String address, int lastByte, String messege, String response) {
  // Serial.println("OnEvent : [ A : " + address + " | D : " + String(lastByte, HEX) + " | M : " + messege + " | R : " + response + " ]");
  if(digitalRead(LED_BUILTIN) == 1){
    digitalWrite(LED_BUILTIN, 0);
  }
  else {
    digitalWrite(LED_BUILTIN, 1);
  }
}

void uartHMIReceiveCB(void){
  hmi.listen();
}
void uartReceiveCB(void) {
  String send = Serial.readString();
  hmi.setText(_VPAddressNhietDoThucTe, send);
}

void setup() {
  Serial.begin(115200);
  Serial.println("DWIN HMI ~ Hello World");
  Serial.onReceive(uartReceiveCB, true); // chạy CB khi hết time out
  pinMode(LED_BUILTIN, OUTPUT);

  Serial2.onReceive(uartHMIReceiveCB, true);
  hmi.echoEnabled(false);
  hmi.hmiCallBack(onHMIEvent);
  hmi.setPage(130);
}

void loop() {
  // Listen HMI Events
  // Serial.printf("tao rảnh\n");
  delay(1);
}