#include "00_ChuongTrinhChinh.h"
#include "01_Flags.h"
#include "Door.h"
#include "ESPNOW2Ways.h"
#include "Config.h"

//*thay đổi tên dự án ở đây
#define _TEN_DU_AN "KingShakIncuCO2"

//* tự động thay đổi tên version code theo ngày và giờ tại thời điểm biên dịch code
#define _VERSION_CODE LEVEL_CHUYENCHUOI(LEVEL_GHEPCHUOI(_TEN_DU_AN, GHEP_CHUOI(__DATE__, __TIME__)))

#include "07_Heater.h"

//---------------------------------------------------------//
#define _ON 1           // ON thiết bị
#define _OFF 0          // OFF thiết bị
#define _DOTHI_NHIET 1  // Đồ thị nhiệt
#define CORE_1 1        //! hiện tại có 3 task đang chạy ở core 1 ( post - get - ktra wifi)
#define CORE_0 0        //! version hiện tại có 4 task chạy ở core 0(nhiệt + DWIN + CO2 + loop)
#define PIN_RELAY RELAY_PIN
//---------------------------------------------------------//

#define _SIZE_TASK_NHIET 4096
#define _SIZE_TASK_DIEUKHIENQUAT 2048

typedef enum {
  GET_SENSOR = 0,
  GET_TRIAC,
  GET_MAC,
  GET_MAX,

  SET_SETPOINT,
  SET_CALIB,
  SET_MAX,
} SPI_cmd_e;

static const char* debugString[] = {
  "get sensor",
  "get triac",
  "get mac",
  "get max",

  "set setpoint",
  "set calib",
  "set max"
};
void callBack(void* dataFromMaster);
void detecCMD( char*);
//--------------------------------------------------------------------------------------------------------//
Flags _Flags;                // Cờ định thời gian thực thi cách lệnh.
ESPNOW _ESPNow;
DOOR _DOOR;
HEATER _HEATER;  // Điều khiển nhiệt (đọc cảm biến nhiệt và điều khiển PID thanh gia nhiệt)

ESPNOW::DuLieuNhietDoTruyenNhan_t DuLieuGuiDi;
// String SSID = _ESPNow.DuLieuNhanDuocQuaESPNow.TenWifi;
//--------------------------------------------------------------------------------------------------------//
String _ID;  // Số ID của ESP32, đây là số IMEI của board.
int _CODE;   // Mã CODE gửi từ app xuống board để thực thi các tác vụ.
int _MODE;   // Chế độ làm việc của máy.

bool _CoLayDuocSoMacMaster = false;  // cờ lấy được số mac từ master

int m = 1;
bool FlagNhanNhietBanDau = true;
bool FlagBua1LanCuaThao = true;


void KhoiTao(void) {
#pragma region KhoiTao
  //MMMMMMMMM7MMMMMMMMMMMMMMMMMưMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM//
#pragma region Các khởi tạo cơ bản của máy
  //----------------------------------------------------------------------------------------
  // Khởi tạo giao tiếp SERIAL
  Serial.begin(115200);
  Serial.println("");
  //* thời điểm biên dịch chương trình
  Serial.println(__DATE__);
  Serial.println(__TIME__);
  pinMode(RELAY_PIN, OUTPUT);
  //---------------------------------------------------------------
  // Để có thể kết nối với các mô-đun đọc cảm biến SHT3x,
  // mô-đun thời gian thực, mô-đun mở rộng port PCF8574
  // thì trước tiên cần phải khởi tạo giao tiếp I2C
  // (Start the I2C interface firtly).
  Wire.begin();
  //---------------------------------------------------------------

  // Khởi tạo bộ nhớ ROM của ESP32
  EEPROM.begin(512);
  delay(10);

  pinMode(PIN_RELAY, OUTPUT);
  digitalWrite(PIN_RELAY, 0);

  _DOOR.KhoiTao();

  //----------------------------------------------------------------------------------------
  // Xóa trắng thông số cài đặt cho board.
#ifdef XOA_THONG_SO_BOARD
  _ThongSoBoard.XoaEEPROM(0, _ChieuDaiMaxThongSoBoard);
#endif

#pragma region Khởi tạo điều khiển nhiệt
  //----------------------------------------------------------------------------------------
  _HEATER.KhoiTao();
  //----------------------------------------------------------------------------------------
#pragma endregion Khởi tạo điều khiển nhiệt

#pragma region ESPNow
  //--------------------------------------------------------------------------------------------
  // if (strstr(SSID.c_str(), "") != 0) {
  //   _ESPNow.setupWiFiChannel(SSID);
  // }
  _ESPNow.KhoiTaoESPNow();
  //--------------------------------------------------------------------------------------------

  if (_DOOR.TrangThai() == DOOR_OPEN) {
    DuLieuGuiDi.TrangThaiCua = 2;
    _DOOR.CoDongMoCua = DOOR_OPEN;
  }
  else if (_DOOR.TrangThai() == DOOR_CLOSE) {
    DuLieuGuiDi.TrangThaiCua = 1;
    _DOOR.CoDongMoCua = DOOR_CLOSE;
  }
#pragma endregion ESPNow
#pragma endregion KhoiTao
}

void ChayChuongTrinhChinh(void) {
#pragma region ChayChuongTrinhChinh
  //===================================================================================
  //--------------- Begin: THỰC THI CHƯƠNG TRÌNH CHÍNH ------------------------------//
  //===================================================================================
  // Bật các cờ lấy mốc thời gian thực hiện các tác vụ.
  // Luôn luôn gọi ở đầu vòng loop().
  _Flags.TurnONFlags();

  ThucThiTacVuTheoFLAG();

  // Tắt các cờ lấy mốc thời gian thực hiện các tác vụ.
  // Luôn luôn gọi ở cuối vòng loop().
  _Flags.TurnOFFFlags();

  if (Serial.available()) {
    String str = Serial.readString();
    detecCMD(( char*)str.c_str());
  }

  //===================================================================================
  //--------------- End: THỰC THI CHƯƠNG TRÌNH CHÍNH --------------------------------//
  //===================================================================================
  delay(1);  //! đảm bảo chương trình ko bị watch dog reset vì chưa rỏ nguyên nhân do đâu
#pragma endregion ChayChuongTrinhChinh
}

//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
//============================ Begin: CÁC HÀM THỰC THI TÁC VỤ THEO FLAG =============================//
//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
#pragma region CÁC HÀM THỰC THI TÁC VỤ THEO FLAG
void ThucThiTacVuTheoFLAG(void) {
#pragma region ThucThiTacVuTheoFLAG
  //------------------------------------------------------------------------------
#pragma region Flag1s
#ifdef _Flag_1s
  if (_Flags.Flag.t1s) {
    // Nhận
    _HEATER.ThoiGianBatQuat = _ESPNow.DuLieuNhanDuocQuaESPNow.ThoiGianBatQuat = 1;
    _HEATER.ThoiGianTatQuat = _ESPNow.DuLieuNhanDuocQuaESPNow.ThoiGianTatQuat = 0;
    _HEATER.HeSoCalib = _ESPNow.DuLieuNhanDuocQuaESPNow.HeSoCalibNhietDo = 0;

    static float temp = _ESPNow.DuLieuNhanDuocQuaESPNow.fSetpointNhiet;
    if (_ESPNow.DuLieuNhanDuocQuaESPNow.fSetpointNhiet > 0 && _ESPNow.DuLieuNhanDuocQuaESPNow.fSetpointNhiet != temp) {
      _HEATER.fSetpointNhiet = _ESPNow.DuLieuNhanDuocQuaESPNow.fSetpointNhiet;
      _HEATER.CaiDatNhietDo(_HEATER.fSetpointNhiet);
      temp = _ESPNow.DuLieuNhanDuocQuaESPNow.fSetpointNhiet;
    }

    if (_ESPNow.DuLieuNhanDuocQuaESPNow.fSetpointChoCua >= 0) {
      _HEATER.fSetpointChoCua = _ESPNow.DuLieuNhanDuocQuaESPNow.fSetpointChoCua;
    }

    // // xử lý tắt gia nhiệt nhưng ko tắt điều khiển quạt
    // if (_ESPNow.DuLieuNhanDuocQuaESPNow.fSetpointNhiet == 0) {
    //   _HEATER.fSetpointNhiet = _ESPNow.DuLieuNhanDuocQuaESPNow.fSetpointNhiet;
    //   _HEATER.CaiDatNhietDo(_HEATER.fSetpointNhiet);
    //   _HEATER.fSetpointChoCua = 0.0f;
    // }

    // //xử lý chế độ tiệt trùng, nhiệt độ cài lớn hơn 60 độ
    // if (_ESPNow.DuLieuNhanDuocQuaESPNow.fSetpointNhiet > 60.0f) {
    //   _HEATER.fSetpointChoCua = 0.0f;
    // }

    // Gửi

    DuLieuGuiDi.NhietDo = _HEATER.LayNhietDoLoc();
    DuLieuGuiDi.NhietDo_VANH = _HEATER.LayNhietDoLoc_VANH();
    DuLieuGuiDi.NhietDo_CUA = _HEATER.LayNhietDoLoc_CUA();
    DuLieuGuiDi.fanPower = _HEATER.fanPower;                            // Thay đổi icon quạt
    // DuLieuGuiDi.LayThoiGianKichTriac = _HEATER.LayThoiGianKichTriac();  // Thay đổi icon nhiệt

    if (_DOOR.TrangThai() != _DOOR.CoDongMoCua) {
      /* Mở cửa */
      if (_DOOR.TrangThai() == DOOR_OPEN) {
        DuLieuGuiDi.TrangThaiCua = 2;
        _DOOR.CoDongMoCua = DOOR_OPEN;

        _HEATER.TatDieuKhienNhietDo();
        digitalWrite(PIN_RELAY, 0);
      }

      /* Đóng cửa */
      else if (_DOOR.TrangThai() == DOOR_CLOSE) {
        DuLieuGuiDi.TrangThaiCua = 1;
        _DOOR.CoDongMoCua = DOOR_CLOSE;

        if (_ESPNow.DuLieuNhanDuocQuaESPNow.TrangThaiONOFF == 1) {
          _HEATER.BatDieuKhienNhietDo();
          digitalWrite(PIN_RELAY, 1);
        }
      }
    }
  }
#endif

#pragma endregion ThucThiTacVuTheoFLAG
}
#pragma endregion CÁC HÀM THỰC THI TÁC VỤ THEO FLAG
//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
//============================ End: CÁC HÀM THỰC THI TÁC VỤ THEO FLAG ===============================//
//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM

void detecCMD(char* str) {
  float value = 0;
  if (strstr(str, "ON") && _DOOR.TrangThai() == DOOR_CLOSE) {
    _ESPNow.DuLieuNhanDuocQuaESPNow.TrangThaiONOFF = 1;
    Serial.println("ON");
    _HEATER.BatDieuKhienNhietDo();
    digitalWrite(PIN_RELAY, 1);
  }
  else if (strstr(str, "OFF")) {
    _ESPNow.DuLieuNhanDuocQuaESPNow.TrangThaiONOFF = 0;
    Serial.println("OFF");
    _HEATER.TatDieuKhienNhietDo();
    digitalWrite(PIN_RELAY, 0);
  }
  else if (strstr(str, "buog")) {
    sscanf(str, "buog:%f", &value);
    _ESPNow.DuLieuNhanDuocQuaESPNow.fSetpointNhiet = value;
    Serial.printf("setpoint buồng %f\n", _ESPNow.DuLieuNhanDuocQuaESPNow.fSetpointNhiet);
  }
  else if (strstr(str, "offset")) {
    sscanf(str, "offset:%f", &value);
    _ESPNow.DuLieuNhanDuocQuaESPNow.offset = value;
    _HEATER.offset = value;
    Serial.printf("offset %f\n", _ESPNow.DuLieuNhanDuocQuaESPNow.offset);
  }
  else if (strstr(str, "cua")) {
    sscanf(str, "cua:%f", &value);
    _ESPNow.DuLieuNhanDuocQuaESPNow.fSetpointChoCua = value;
    Serial.printf("setpoint cửa %f\n", _ESPNow.DuLieuNhanDuocQuaESPNow.fSetpointChoCua);
  }
}
