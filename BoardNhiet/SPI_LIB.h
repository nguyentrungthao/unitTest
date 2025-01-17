/**
 *@file SPI_LIB.h
 * @author Trung Thao
 * @brief
 * @version 0.1
 * @date 2024-08-23
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef _SPI_LIB_H_
#define _SPI_LIB_H_

#include <Arduino.h>
#include <vector>
#include <String.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "esp_check.h"
#include "driver/gpio.h"
#include "SPI.h"

#include "driver/spi_slave.h"
#include "driver/spi_master.h"

#include "Config.h"

#define SPI_PARAM_LOCK(xMutexKey)    do {} while (xSemaphoreTake(xMutexKey, portMAX_DELAY) != pdPASS)
#define SPI_PARAM_UNLOCK(xMutexKey)  xSemaphoreGive(xMutexKey)

 // nếu muốn debug tool -> core debug level -> Verbone ở Arduino
#define SIZE_OF_BUFFER 129
#define EVENT_GROUP_RECEIVE_SPI (1 << 0)

typedef void (*callBackRespone_t)(void* pvInput);

class SlaveSPI {
public:
    SlaveSPI(int8_t csPin = -1, int8_t sckPin = -1,
        int8_t misoPin = -1, int8_t mosiPin = -1, spi_host_device_t xSPIHost = (spi_host_device_t)HSPI, uint8_t mode = 0, uint8_t queueSize = 1,
        slave_transaction_cb_t post_setup_cb = NULL, slave_transaction_cb_t post_trans_cb = NULL) :
        m_i8CS(csPin), m_i8Sck(sckPin), m_i8Mosi(mosiPin), m_i8Miso(misoPin), m_xSPIHost(xSPIHost), m_u8Mode(mode),
        m_u8QueueSize(queueSize), m_xPost_setup_cb(post_setup_cb), m_xPost_trans_cb(post_trans_cb) {
    };
    ~SlaveSPI() {
        if (m_pu8Recvbuf) {
            free(m_pu8Recvbuf);
        }
        if (m_pu8sendbuf) {
            free(m_pu8Recvbuf);
        }
    };

    void begin(uint16_t sizeBuf = SIZE_OF_BUFFER, uint8_t priorityResponeTask = 4);
    void begin(uint8_t* pcSendBuf, uint16_t u16SizeSendvBuf, uint8_t* pcRecvbuf, uint16_t u16SizeRecvBuf, uint8_t priorityResponeTask = 4);
    void addCallbackFunc(callBackRespone_t callBack);

    void sendData(String Sendata);
    // void receiveData(); // chưa biết viết như nào 

    uint8_t* u8GetRecvBuf() { return m_pu8Recvbuf; }
    uint8_t* u8GetSendBuf() { return m_pu8sendbuf; }

    static void taskResponeSPI(void* ptr);
    static void IRAM_ATTR DataComing(void* ptr);

private:
    // các tham số cấu hình được 
    int8_t m_i8CS, m_i8Sck, m_i8Mosi, m_i8Miso;
    callBackRespone_t m_xCallBackFuncResponeMaster;
    void* m_pvInputParameterCallBackFunc, * m_pvOutputParameterCallBackFunc;
    uint8_t m_u8QueueSize, m_u8Mode;
    slave_transaction_cb_t m_xPost_setup_cb; // hàm tự động gọi khi ghi dữ liệu mới vào thành ghi của slave SPI 
    slave_transaction_cb_t m_xPost_trans_cb; // hàm tự động gọi khi truyền nhận SPI xog 
    uint8_t* m_pu8Recvbuf;
    uint8_t* m_pu8sendbuf;
    uint16_t m_u16SizeSendvBuf, m_u16SizeRecvBuf;
    spi_host_device_t m_xSPIHost;

    // các tham số class tự cấu hình
    spi_slave_transaction_t m_xSlaveTransactrion;
    TaskHandle_t m_xTaskHandleResponeSPI;
    EventGroupHandle_t m_xEventGroupHandle;
    // SemaphoreHandle_t  m_xMutex; //! CHƯA BIẾT cần bảo vệ m_pu8Recvbuf và m_pu8sendbuf khi đang gửi nhận SPI KHÔNG?
};

class MasterSPI {
public:
    MasterSPI(int8_t sckPin = -1,
        int8_t misoPin = -1, int8_t mosiPin = -1, spi_host_device_t xSPIHost = (spi_host_device_t)HSPI, uint8_t mode = 0, uint8_t queueSize = 1, uint32_t u32ClockSpeedHz = 1000000) :
        m_i8Sck(sckPin), m_i8Mosi(mosiPin), m_i8Miso(misoPin), m_xSPIHost(xSPIHost), m_u8Mode(mode), m_u8QueueSize(queueSize), m_u32ClockSpeedHz(u32ClockSpeedHz)
    {
    }
    ~MasterSPI() {
        if (m_pu8Recvbuf) {
            free(m_pu8Recvbuf);
        }
    }

    void begin(uint16_t u16SizeRecvBuf = SIZE_OF_BUFFER);
    void begin(uint8_t* pcRecvbuf, uint16_t u16SizeRecvBuf);

    void addSlave(int8_t i8CSpin = -1);
    void deleteSlave(uint8_t ui8CSpin);
    uint8_t getNumberOfSlave() { return m_xCsList.size(); }
    // bool checkSlave(); //! check sự tồn tại của slave CHƯA NGHĨ RA CÁCH 

    uint8_t* getData(int8_t ui8CSpin, uint16_t cmdCode);
    void setData(int8_t ui8CSpin, uint16_t cmdCode, String data);
    uint8_t* getData(int8_t ui8CSpin, String sendString);
    void setData(int8_t ui8CSpin, String sendString);


    //các hàm chức năng
    uint8_t* u8GetRecvBuf() { return m_pu8Recvbuf; }
    void beginTransaction(int8_t i8CSpin = -1);
    void endTransaction(int8_t i8CSpin = -1);
    void sendCmd(String string);
    void readData(uint16_t sizeRecvBuf = SIZE_OF_BUFFER);

private:
    // các tham số cấu hình được
    int8_t m_i8Sck, m_i8Mosi, m_i8Miso;
    std::vector<int8_t> m_xCsList;
    uint8_t m_u8QueueSize, m_u8Mode;
    uint32_t m_u32ClockSpeedHz;
    uint8_t* m_pu8Recvbuf;
    uint16_t m_u16SizeRecvBuf;
    spi_host_device_t m_xSPIHost;

    // các tham số class tự cấu hình
    spi_device_handle_t m_xSPIMasterHanler;
    spi_transaction_t m_xMasterTransactrion;
    SemaphoreHandle_t m_xMutex; //! bảo vệ SPI khi dang giao tiếp 1 slave
};


#endif //_SPI_LIB_H_