/**
 *@file SPI_LIB.cpp
 * @author Trung Thao
 * @brief
 * @version 0.1
 * @date 2024-08-23
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "SPI_LIB.h"

#define TAG "SPI_LIB"

#pragma region CODE SLAVE
 /**
   *@brief
   *
   * @param sizeBuf    kích thước của cả bộ nhớ gửi và nhận dữ liệu, sẽ được cấp phát động trong class
   * @param priorityResponeTask độ ưu tiên của task phản hồi khi có yêu cầu từ master
   */
void SlaveSPI::begin(uint16_t sizeBuf, uint8_t priorityResponeTask) {
  //check các tham số cấu hình
  if (m_i8Mosi < 0 || m_i8Miso < 0 || m_i8Sck < 0 || m_i8CS < 0) {
    Serial.printf("%s invalib pin\n", __func__);
  }

  spi_bus_config_t buscfg = {
    .mosi_io_num = m_i8Mosi,
    .miso_io_num = m_i8Miso,
    .sclk_io_num = m_i8Sck,
    .quadwp_io_num = -1,
    .quadhd_io_num = -1,
  };

  //Configuration for the SPI slave interface
  spi_slave_interface_config_t slvcfg = {
    .spics_io_num = m_i8CS,
    .flags = 0,
    .queue_size = m_u8QueueSize,
    .mode = m_u8Mode,
    .post_setup_cb = m_xPost_setup_cb,
    .post_trans_cb = m_xPost_trans_cb
  };

  // cấu hình kéo lên cho tất cả các chân, tránh nhiễu khi không kết nối master
  gpio_set_pull_mode((gpio_num_t)m_i8Miso, GPIO_PULLUP_ONLY);
  gpio_set_pull_mode((gpio_num_t)m_i8Mosi, GPIO_PULLUP_ONLY);
  gpio_set_pull_mode((gpio_num_t)m_i8CS, GPIO_PULLUP_ONLY);
  // khởi tạo ngoại vi slave SPI
  ESP_ERROR_CHECK(spi_slave_initialize(m_xSPIHost, &buscfg, &slvcfg, SPI_DMA_CH_AUTO));

  //cấp phát bộ nhớ nhận giử
  m_u16SizeSendvBuf = m_u16SizeRecvBuf = sizeBuf;
  m_pu8Recvbuf = (uint8_t*)calloc(m_u16SizeRecvBuf, sizeof(uint8_t));
  m_pu8sendbuf = (uint8_t*)calloc(m_u16SizeSendvBuf, sizeof(uint8_t));

  // cấu hình dữ liệu truyền nhận
  memset(&m_xSlaveTransactrion, 0, sizeof(m_xSlaveTransactrion));
  m_xSlaveTransactrion.length = sizeBuf * 8;  // nhân 8 vì nó tính theo bit
  m_xSlaveTransactrion.tx_buffer = m_pu8sendbuf;
  m_xSlaveTransactrion.rx_buffer = m_pu8Recvbuf;

  m_xEventGroupHandle = xEventGroupCreate();
  // m_xMutex = xSemaphoreCreateMutex();
  xTaskCreatePinnedToCore(taskResponeSPI, "task respone SPI", 2048, (void*)this, priorityResponeTask, &m_xTaskHandleResponeSPI, ARDUINO_EVENT_RUNNING_CORE);
  attachInterruptArg(m_i8CS, DataComing, (void*)this, FALLING);
  SerialPrintf("%s SPI SLAVE khởi tạo thành công\n", __func__);
}
/**
 *@brief
 *
 * @param pcRecvbuf vùng nhớ chứa dữ liệu khi master ghi dữ liệu về
 * @param pcSendBuf dữ liệu gửi đi khi master yêu cầu dữ liệu
 * @param priorityResponeTask độ ưu tiên của task phản hồi khi có yêu cầu từ master
 */
void SlaveSPI::begin(uint8_t* pcSendBuf, uint16_t u16SizeSendvBuf, uint8_t* pcRecvbuf, uint16_t u16SizeRecvBuf, uint8_t priorityResponeTask) {
  // delay bỏ qua giai đoạn boot nhiễu của esp master, nếu master kéo CS bậy khiến cho slave hiểu nhầm đã giao tiếp
  // delay(5000);
  if (pcSendBuf == NULL) {
    Serial.printf("%s pcSendBuf is null\n", __func__);
    return;
  }
  if (pcRecvbuf == NULL) {
    Serial.printf("%s pcRecvbuf is null\n", __func__);
    return;
  }
  //check các tham số cấu hình
  if (m_i8Mosi < 0 || m_i8Miso < 0 || m_i8Sck < 0 || m_i8CS < 0) {
    Serial.printf("%s invalib pin\n", __func__);
    return;
  }

  spi_bus_config_t buscfg = {
    .mosi_io_num = m_i8Mosi,
    .miso_io_num = m_i8Miso,
    .sclk_io_num = m_i8Sck,
    .quadwp_io_num = -1,
    .quadhd_io_num = -1,
  };

  //Configuration for the SPI slave interface
  spi_slave_interface_config_t slvcfg = {
    .spics_io_num = m_i8CS,
    .flags = 0,
    .queue_size = m_u8QueueSize,
    .mode = m_u8Mode,
    .post_setup_cb = m_xPost_setup_cb,
    .post_trans_cb = m_xPost_trans_cb
  };

  // cấu hình kéo lên cho tất cả các chân, tránh nhiễu khi không kết nối master
  gpio_set_pull_mode((gpio_num_t)m_i8Miso, GPIO_PULLUP_ONLY);
  gpio_set_pull_mode((gpio_num_t)m_i8Mosi, GPIO_PULLUP_ONLY);
  gpio_set_pull_mode((gpio_num_t)m_i8CS, GPIO_PULLUP_ONLY);
  // khởi tạo ngoại vi slave SPI
  ESP_ERROR_CHECK(spi_slave_initialize(m_xSPIHost, &buscfg, &slvcfg, SPI_DMA_CH_AUTO));


  //cấp phát bộ nhớ nhận giử
  m_pu8Recvbuf = pcRecvbuf;
  m_pu8sendbuf = pcSendBuf;

  // cấu hình dữ liệu truyền nhận
  memset(&m_xSlaveTransactrion, 0, sizeof(m_xSlaveTransactrion));
  m_xSlaveTransactrion.length = u16SizeSendvBuf * 8;  // nhân 8 vì nó tính theo bit
  m_xSlaveTransactrion.tx_buffer = m_pu8sendbuf;
  m_xSlaveTransactrion.rx_buffer = m_pu8Recvbuf;
  memset(m_pu8sendbuf, 0, u16SizeSendvBuf);
  memset(m_pu8Recvbuf, 0, u16SizeRecvBuf);

  m_xEventGroupHandle = xEventGroupCreate();
  xTaskCreatePinnedToCore(taskResponeSPI, "task respone SPI", 2048, (void*)this, priorityResponeTask, &m_xTaskHandleResponeSPI, ARDUINO_EVENT_RUNNING_CORE);
  attachInterruptArg(m_i8CS, DataComing, (void*)this, FALLING);
  SerialPrintf("%s SPI SLAVE khởi tạo thành công\n", __func__);
}


void SlaveSPI::sendData(String Sendata) {
  if (Sendata.length() > 128) {
    Serial.printf("\n\t\tCHUỖI DÀI HƠN 128\n");
  }
  m_xSlaveTransactrion.length = 128 * 8;
  m_xSlaveTransactrion.tx_buffer = Sendata.c_str();
  SerialPrintf("%s master send: %s\n", __func__, Sendata.c_str());
  spi_slave_transmit(m_xSPIHost, &m_xSlaveTransactrion, portMAX_DELAY);
}

/**
 *@brief cài hàm call back để gọi khi có yêu cầu dữ liệu từ master
 *
 * @param calBack địa chỉ hàm callback
 * @param pvOutputParameterCallBackFunc
 */
void SlaveSPI::addCallbackFunc(callBackRespone_t callBack) {
  if (callBack != NULL) {
    m_xCallBackFuncResponeMaster = callBack;
  }
}

/**
 *@brief task phản hồi khi master yêu cầu dữ liệu
 *
 * @param ptr
 */
void SlaveSPI::taskResponeSPI(void* ptr) {
  SlaveSPI* pxSlaveSPI = (SlaveSPI*)ptr;

  while (1) {
    xEventGroupWaitBits(pxSlaveSPI->m_xEventGroupHandle, EVENT_GROUP_RECEIVE_SPI, pdTRUE, pdTRUE, portMAX_DELAY);
    spi_slave_transmit(pxSlaveSPI->m_xSPIHost, &pxSlaveSPI->m_xSlaveTransactrion, portMAX_DELAY);
    Serial.printf("%s slave Received: %s\n", __func__, pxSlaveSPI->m_pu8Recvbuf);

    // nếu có thêm hàm call back sẽ được gọi
    if (pxSlaveSPI->m_xCallBackFuncResponeMaster != NULL) {
      // gọi hàm call back do người dùng định nghĩa
      pxSlaveSPI->m_xCallBackFuncResponeMaster(pxSlaveSPI->m_pu8Recvbuf);
      memset(pxSlaveSPI->m_pu8Recvbuf, 0, pxSlaveSPI->m_u16SizeRecvBuf);
    }
  }
}

/**
 *@brief hàm ngắt chân Cs để detect master muốn nhận dữ liệu
 *
 * @param ptr
 */
void IRAM_ATTR SlaveSPI::DataComing(void* ptr) {
  SlaveSPI* pxSlaveSPI = (SlaveSPI*)ptr;

  //Sometimes due to interference or ringing or something, we get two irqs after eachother. This is solved by
  //looking at the time between interrupts and refusing any interrupt too close to another one.
  static uint32_t lasthandshaketime_us;
  uint32_t currtime_us = esp_timer_get_time();
  uint32_t diff = currtime_us - lasthandshaketime_us;
  if (diff < 500) {
    return;  //ignore everything <1ms after an earlier irq
  }
  lasthandshaketime_us = currtime_us;

  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xEventGroupSetBitsFromISR(pxSlaveSPI->m_xEventGroupHandle, EVENT_GROUP_RECEIVE_SPI, &xHigherPriorityTaskWoken);
}

#pragma region CODE MASTER

void MasterSPI::begin(uint16_t u16SizeRecvBuf) {
  //Configuration for the SPI bus
  spi_bus_config_t buscfg = {
    .mosi_io_num = m_i8Mosi,
    .miso_io_num = m_i8Miso,
    .sclk_io_num = m_i8Sck,
    .quadwp_io_num = -1,
    .quadhd_io_num = -1
  };
  ESP_ERROR_CHECK(spi_bus_initialize(m_xSPIHost, &buscfg, SPI_DMA_CH_AUTO));

  //Configuration for the SPI device on the other side of the bus
  spi_device_interface_config_t devcfg = {
    .command_bits = 0,
    .address_bits = 0,
    .dummy_bits = 0,
    .mode = m_u8Mode,
    .duty_cycle_pos = 128,  //50% duty cycle
    .cs_ena_posttrans = 4,  //Keep the CS low 3 cycles after transaction, to stop slave from missing the last bit when CS has less propagation delay than CLK
    .clock_speed_hz = m_u32ClockSpeedHz,
    // .spics_io_num = GPIO_CS,
    .queue_size = m_u8QueueSize
  };
  spi_bus_add_device(m_xSPIHost, &devcfg, &m_xSPIMasterHanler);

  m_u16SizeRecvBuf = u16SizeRecvBuf;
  m_pu8Recvbuf = (uint8_t*)calloc(m_u16SizeRecvBuf, sizeof(uint8_t));

  memset(&m_xMasterTransactrion, 0, sizeof(m_xMasterTransactrion));
  m_xMasterTransactrion.rx_buffer = m_pu8Recvbuf;

  m_xMutex = xSemaphoreCreateMutex();
  SerialPrintf("%s SPI MASTER khởi tạo thành công\n", __func__);
}

void MasterSPI::begin(uint8_t* pcRecvbuf, uint16_t u16SizeRecvBuf) {
  //Configuration for the SPI bus
  spi_bus_config_t buscfg = {
    .mosi_io_num = m_i8Mosi,
    .miso_io_num = m_i8Miso,
    .sclk_io_num = m_i8Sck,
    .quadwp_io_num = -1,
    .quadhd_io_num = -1
  };
  ESP_ERROR_CHECK(spi_bus_initialize(m_xSPIHost, &buscfg, SPI_DMA_CH_AUTO));

  //Configuration for the SPI device on the other side of the bus
  spi_device_interface_config_t devcfg = {
    .command_bits = 0,
    .address_bits = 0,
    .dummy_bits = 0,
    .mode = m_u8Mode,
    .duty_cycle_pos = 128,  //50% duty cycle
    .cs_ena_posttrans = 4,  //Keep the CS low 3 cycles after transaction, to stop slave from missing the last bit when CS has less propagation delay than CLK
    .clock_speed_hz = m_u32ClockSpeedHz,
    // .spics_io_num = GPIO_CS,
    .queue_size = m_u8QueueSize
  };
  spi_bus_add_device(m_xSPIHost, &devcfg, &m_xSPIMasterHanler);

  m_u16SizeRecvBuf = u16SizeRecvBuf;
  memset(&m_xMasterTransactrion, 0, sizeof(m_xMasterTransactrion));
  m_xMasterTransactrion.rx_buffer = m_pu8Recvbuf = pcRecvbuf;

  m_xMutex = xSemaphoreCreateMutex();
  SerialPrintf("%s SPI MASTER khởi tạo thành công\n", __func__);
}

void MasterSPI::addSlave(int8_t i8CSpin) {
  if (i8CSpin < 0) {
    Serial.printf("%s giá trị không hợp lệ\n", __func__);
    return;
  }
  m_xCsList.push_back(i8CSpin);
  pinMode(i8CSpin, OUTPUT);
  digitalWrite(i8CSpin, 1);
}
void MasterSPI::deleteSlave(uint8_t ui8CSpin) {
  for (int i = m_xCsList.size() - 1; i >= 0; i--) {
    if (m_xCsList[i] == ui8CSpin) {
      m_xCsList.erase(m_xCsList.begin() + i);
    }
  }
}

void MasterSPI::beginTransaction(int8_t i8CSpin) {
  if (i8CSpin < 0) {
    Serial.printf("%s giá trị không hợp lệ\n", __func__);
    return;
  }
  for (int i = m_xCsList.size() - 1; i >= 0; i--) {
    if (m_xCsList[i] == i8CSpin) {
      SPI_PARAM_LOCK(m_xMutex);
      digitalWrite(i8CSpin, 0);
      delay(10);  // chờ cho slave chuẩn bị dữ liệu
      break;
    }
  }
}
void MasterSPI::endTransaction(int8_t i8CSpin) {
  if (i8CSpin < 0) {
    Serial.printf("%s giá trị không hợp lệ\n", __func__);
    return;
  }
  for (int i = m_xCsList.size() - 1; i >= 0; i--) {
    if (m_xCsList[i] == i8CSpin) {
      digitalWrite(i8CSpin, 1);
      SPI_PARAM_UNLOCK(m_xMutex);
      break;
    }
  }
}
void MasterSPI::sendCmd(String string) {
  // m_xMasterTransactrion.length = string.length() * 8; //!<---- éo hiểu tại sao phải 128byte thì mới chạy đc
  if (string.length() > 128) {
    Serial.printf("\n\t\tCHUỖI DÀI HƠN 128\n");
  }
  m_xMasterTransactrion.length = 128 * 8;
  m_xMasterTransactrion.tx_buffer = string.c_str();
  SerialPrintf("%s master send: %s\n", __func__, string.c_str());
  spi_device_transmit(m_xSPIMasterHanler, &m_xMasterTransactrion);
}
void MasterSPI::readData(uint16_t sizeRecvBuf) {
  // chuẩn bị dữ liệu giả để gửi cùng khi đọc 
  uint8_t* txString = (uint8_t*)malloc(sizeRecvBuf * sizeof(uint8_t));
  memset(txString, 0xFF, sizeRecvBuf);
  m_xMasterTransactrion.length = sizeRecvBuf * 8;
  m_xMasterTransactrion.tx_buffer = txString;

  //xóa buffer trước khi đọc từ slave  
  memset(m_pu8Recvbuf, 0, m_u16SizeRecvBuf);

  spi_device_transmit(m_xSPIMasterHanler, &m_xMasterTransactrion);

  SerialPrintf("%s master read: %s\n", __func__, m_pu8Recvbuf);
  free(txString);
}

uint8_t* MasterSPI::getData(int8_t ui8CSpin, uint16_t cmdCode) {
  String cmd(cmdCode);
  SerialPrintf("%s lệnh gửi cho slave: %s\n", __func__, cmd.c_str());

  beginTransaction(ui8CSpin);
  sendCmd(cmd);
  delay(30);  // chờ slave xử lý gửi dữ liệu, SẼ HAY HƠN NẾU CÓ CHÂN NGẮT TỪ SLAVE CHO MASTER
  readData();
  endTransaction(ui8CSpin);

  return m_pu8Recvbuf;
}
void MasterSPI::setData(int8_t ui8CSpin, uint16_t cmdCode, String data) {
  String cmd(cmdCode);
  cmd = cmd + ":" + data;
  SerialPrintf("%s lệnh gửi cho slave: %s\n", __func__, cmd.c_str());

  beginTransaction(ui8CSpin);
  sendCmd(cmd);
  endTransaction(ui8CSpin);
}

uint8_t* MasterSPI::getData(int8_t ui8CSpin, String sendString) {
  SerialPrintf("%s lệnh gửi cho slave: %s\n", __func__, sendString.c_str());

  beginTransaction(ui8CSpin);
  sendCmd(sendString);
  delay(30);  // chờ slave xử lý gửi dữ liệu, SẼ HAY HƠN NẾU CÓ CHÂN NGẮT TỪ SLAVE CHO MASTER
  readData();
  endTransaction(ui8CSpin);

  return m_pu8Recvbuf;
}
void MasterSPI::setData(int8_t ui8CSpin, String sendString) {
  SerialPrintf("%s lệnh gửi cho slave: %s\n", __func__, sendString.c_str());
  beginTransaction(ui8CSpin);
  sendCmd(sendString);
  endTransaction(ui8CSpin);
}
