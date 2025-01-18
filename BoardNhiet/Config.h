/**
 *@file 00_Config.h
 * @author Trung Thao (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2024-08-26
 *
 * @copyright Copyright (c) 2024
 *
 */


#ifndef _CONFIG_H_
#define _CONFIG_H_

 /**
  *@brief các define cấu hình code cho chương trình
  *
 */
#pragma region SOFTWARE
#ifndef _CONFIG_SOFTWARE_
#define _CONFIG_SOFTWARE_

#define DO_UU_TIEN_TASK_NHIET 3
#define PRORITY_RESPONE_TASK 4
 // #define DEBUG //* điều khiển việc log serial
  /*
      KHU VỰC DEFINE ĐIỀU KHIỂN LOG DEBUG
  */

  // Cho một số đoạn Serial.print hiển thị debug.
#define debug //* cho phép tất cả các đoạn log cũ được hoạt động
// #define debug_thongsoboard
#define ON_OFF_DEBUG //* cho phép lựa chọn dùng cờ để bật tắt log debug khi chạy run time - tắt đi nếu muốn không muốn dùng

//*khóa từng khu vực và cần define debug(dòng 9) trước nếu muốn hiển thị
// #define DEBUG_POST_GET //* cho phép log trong task LangNgheLenhAppGuiXuongBoard
// #define DEBUG_WIFI //*cho phép log trong thư viện WIFI
// #define DEBUG_LOG_HMI //* cho phép log trong HMI
#define DEBUG_HEATER //*log trong nhiệt 
#define DEBUG_CO2 //*log trong CO2
// #define DEBUG_RTC //* hiển thị debug RTC DS3231
// #define DEBUG_THONG_SO_BOARD //*
/*
    KHU VỰC DEFINE ĐIỀU KHIỂN LOG DEBUG
*/

//*------------------------------------------------------------------------------------------

/*
    KHU VỰC DEFINE ẢNH HƯỞNG ĐẾN CODE TRONG CHƯƠNG TRÌNH
*/
//* tắt define để back lại code calib nhiệt cũ calib 1 giá trị cho tất cả
#define CALIB_NHIEU_DIEM_NHIET 

//* khu vực define chạy thử nếu không có code wifi
#define BAT_TAT_WIFI

//* hiển thị trang cảnh báo 
// #define CANH_BAO

//*define dành cho DEV
#define NHAN_LENH_TU_UART
// #define KHONG_LAM_TRON_SO
#define BAT_TAT_QUAT_BANG_SERIAL
// #define XOA_THONG_SO_BOARD

// #define TEST_CONG_THUC
// #define BO_BOARD_CO2
#define NHIET_DO_TEST 37
#define SETPOINT_TEST 42

/*
    KHU VỰC DEFINE ẢNH HƯỞNG ĐẾN CODE TRONG CHƯƠNG TRÌNH
*/


#if defined(DEBUG)
#define SerialPrintf(string, ...) Serial.printf(string, ##__VA_ARGS__)
#define SerialPrintln(string, ...) Serial.println(string, ##__VA_ARGS__)
#define SerialPrint(string, ...) Serial.print(string, ##__VA_ARGS__)
#else //defined(DEBUG)
#define SerialPrintf(string, ...) (void*)0
#define SerialPrintln(string, ...) (void*)0
#define SerialPrint(string, ...) (void*)0
#endif //defined(DEBUG)

#endif //_CONFIG_SOFTWARE_
#pragma endregion

/**
 *@brief các define chân theo phần cứng hiện cho khung code
 *
*/
#pragma region HARDWARE 
#ifndef _CONFIG_HARDWARE_
#define _CONFIG_HARDWARE_

#include "AnhLABV01HardWare.h"

#endif //_CONFIG_HARDWARE_
#pragma endregion

#endif //_CONFIG_H_
