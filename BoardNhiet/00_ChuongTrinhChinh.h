#ifndef _ChuongTrinhChinh_h
#define _ChuongTrinhChinh_h

#include "Config.h"  // Có define debug để bật/tắt các debug ra Serial.

//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
//============================== Begin: KHAI BÁO THƯ VIỆN ===========================================//
//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
#include <Arduino.h>
#include <Arduino_JSON.h>  // Thư viện xử lý dữ liệu kiểu JSON
#include <EEPROM.h>        // Thư viện để lưu dữ liệu thông số board vào bộ nhớ ROM
#include <Wire.h>          // Để kết nối I2C với mô-đun RTC (thời gian thực),
                           // mô-đun mở rộng port PCF8574, mô-đun đọc cảm biến nhiệt độ & độ ẩm SHT3x
#include <TimeLib.h>       // Thư viện xử lý các tính toán liên quan thời gian.
//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
//============================== End: KHAI BÁO THƯ VIỆN =============================================//
//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM

#if defined(ON_OFF_DEBUG) && defined(debug)
#define SerialPrintf(flag, string, ...) \
  do { \
    if (flag) \
      Serial.printf(string, ##__VA_ARGS__); \
  } while (0)

#define SerialPrintln(flag, string, ...) \
  do { \
    if (flag) \
      Serial.println(string, ##__VA_ARGS__); \
  } while (0)
#define SerialPrint(flag, string, ...) \
  do { \
    if (flag) \
      Serial.print(string, ##__VA_ARGS__); \
  } while (0)

#elif !defined(ON_OFF_DEBUG) && defined(debug)

#define SerialPrintf(flag, string, ...) Serial.printf(string, ##__VA_ARGS__)
#define SerialPrintln(flag, string, ...) Serial.println(string, ##__VA_ARGS__)
#define SerialPrint(flag, string, ...) Serial.print(string, ##__VA_ARGS__)

#else 

#define SerialPrintf(flag, string, ...) (void*)0
#define SerialPrintln(flag, string, ...) (void*)0
#define SerialPrint(flag, string, ...) (void*)0

#endif

#define GHEP_CHUOI(a, b) a##b
#define LEVEL_GHEPCHUOI(a, b) GHEP_CHUOI(a, b)
#define CHUYENCHUOI(x) #x
#define LEVEL_CHUYENCHUOI(x) CHUYENCHUOI(x)

void KhoiTao(void);               //
void ChayChuongTrinhChinh(void);  //

// Các hàm thực thi tác vụ theo FLAG
//------------------------------------------------------
void ThucThiTacVuTheoFLAG(void);

// Các hàm thực thi tác vụ theo CODE
//------------------------------------------------------

void DieuKhienNhietDo(void);

#endif