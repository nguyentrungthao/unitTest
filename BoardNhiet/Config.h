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

// #define HMI
// #define CO2
// #define WIFI
#define NHIET

#ifdef HMI
#define NAME_VERSION "board màn hình V0.1 trong bộ board các tủ "
//* kết nối các board bằng SPI
#define PIN_SPI_BOARD_SCK 18   // Chân kết nối SCK
#define PIN_SPI_BOARD_MISO 19  // Chân kết nối MISO
#define PIN_SPI_BOARD_MOSI 23  // Chân kết nối MOSI
#define PIN_SPI_SS_NHIET 4     // Chân kết nối Slave3
#define PIN_SPI_SS_CO2 5   // Chân kết nối Slave2
#define PIN_SPI_SS_WIFI 15   // Chân kết nối Slave1

#define PIN_I2C_SDA 21
#define PIN_I2C_SCL 22

#define PIN_DOOR 39
#define PIN_BUZZER 2

#define PIN_HMI_POWER (gpio_num_t)13
#define PIN_HMI_TX (gpio_num_t)17
#define PIN_HMI_RX (gpio_num_t)16

//* kết nối các board với nhau bằng UART
//! có thể sẽ dùng để giao tiếp RS232 với board IO
#define PIN_UART_BOARD_TX (gpio_num_t)33
#define PIN_UART_BOARD_RX (gpio_num_t)32

//* kết nối với IC ethernet ở board IO
#define PIN_SPI_ETHERNET_SCK (gpio_num_t)27
#define PIN_SPI_ETHERNET_MISO (gpio_num_t)15
#define PIN_SPI_ETHERNET_MOSI (gpio_num_t)26
#define PIN_SPI_ETHERNET_SS (gpio_num_t)14


#elif  defined(NHIET)
#define NAME_VERSION "board nhiệt V0.1 trong bộ board các tủ "

//* kết nối các board bằng SPI
#define PIN_SPI_BOARD_MOSI (gpio_num_t)17
#define PIN_SPI_BOARD_MISO (gpio_num_t)16
#define PIN_SPI_BOARD_SCK (gpio_num_t)5
#define PIN_SPI_BOARD_SS (gpio_num_t)34

//* kết nối 3 cảm biến nhiệt trên board 
#define PIN_SPI_MAX31865_SCK (gpio_num_t)18   // Chân kết nối SCK
#define PIN_SPI_MAX31865_MISO (gpio_num_t)19  // Chân kết nối MISO
#define PIN_SPI_MAX31865_MOSI (gpio_num_t)23  // Chân kết nối MOSI
#define PIN_SPI_SS_BUONG (gpio_num_t)14     // Chân kết nối Slave1
#define PIN_SPI_SS_CUA (gpio_num_t)27   // Chân kết nối Slave2
#define PIN_SPI_SS_VACH (gpio_num_t)12   // Chân kết nối Slave3

//* kết nối các board với nhau bằng I2C
#define PIN_I2C_SDA (gpio_num_t)21
#define PIN_I2C_SCL (gpio_num_t)22

#define PIN_DOOR (gpio_num_t)36
#define PIN_ACDET (gpio_num_t)39
#define PIN_SERVO (gpio_num_t)13
#define PIN_TRIAC_BUONG (gpio_num_t)2
#define PIN_TRIAC_QUAT (gpio_num_t)15
#define PIN_TRIAC_CUA (gpio_num_t)26
#define PIN_RELAY (gpio_num_t)25

//* kết nối các board với nhau bằng UART
#define PIN_UART_BOARD_TX (gpio_num_t)33
#define PIN_UART_BOARD_RX (gpio_num_t)32

#elif defined(CO2)
#define NAME_VERSION "Board CO2 trong bộ board các tủ "
//* kết nối các board với nhau bằng SPI   
#define PIN_SPI_BOARD_MOSI (gpio_num_t)23
#define PIN_SPI_BOARD_MISO (gpio_num_t)19
#define PIN_SPI_BOARD_SCK (gpio_num_t)18
#define PIN_SPI_BOARD_SS (gpio_num_t)36 // Chân kết nối Slave1-Master

//* kết nối các board với nhau bằng I2C
#define PIN_I2C_SDA 21
#define PIN_I2C_SCL 22
//* cảm biến CO2 
#define PIN_RS232_TX 17
#define PIN_RS232_RX 16

#define PIN_VAN1 26
#define PIN_VAN2 13
#define PIN_DOOR 39

#elif defined(WIFI)
#define NAME_VERSION "Kit esp32 V1.3 do Uy vẽ "
//* kết nối các board với nhau bằng SPI
#define PIN_SPI_BOARD_MOSI (gpio_num_t)23
#define PIN_SPI_BOARD_MISO (gpio_num_t)19
#define PIN_SPI_BOARD_SCK (gpio_num_t)18
#define PIN_SPI_BOARD_SS (gpio_num_t)36 // Chân kết nối Slave1-Master
//* kết nối các board với nhau bằng UART
#define PIN_UART_BOARD_TX 33
#define PIN_UART_BOARD_RX 32
//* kết nối các board với nhau bằng I2C
#define PIN_I2C_SDA 21
#define PIN_I2C_SCL 22
#else
#error chua chon phan cung
#endif

#endif //_CONFIG_HARDWARE_
#pragma endregion

#endif //_CONFIG_H_
