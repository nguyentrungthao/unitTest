#include "07_Heater.h"

// #define TEST
// #define TEST_TRIAC

static uint8_t chuKi = 0;
static uint16_t iLon_CUA = 0;
static int16_t iNho_CUA = 0;
static bool Laymau = true;
static bool Laymau_CUA = true;
static float fLayNhietDoThucTe(Adafruit_MAX31865& xPt100);

HEATER::HEATER(void)
  : triacBuong((gpio_num_t)TRIAC_BUONG_PIN),
    triacSau((gpio_num_t)TRIAC_SAU_PIN),
    triacCua((gpio_num_t)TRIAC_CUA_PIN),
    triacQuat((gpio_num_t)TRIAC_QUAT_PIN, TIMER_GROUP_1, TIMER_0) {
  this->fanPower = GIA_TRI_BAT_TRIAC_MIN;  // khởi tạo giá trị lớn nhất để triac không kích nữa
  this->HeSoCalib = 0;
  this->trangThaiDieuKhien = _TAT_DIEU_KHIEN_NHIET_DO;
};


void HEATER::KhoiTao() {

  PT100_buong.begin(MAX31865_4WIRE);
  PT100_buong.enable50Hz(true);

  PT100_vanh.begin(MAX31865_4WIRE);
  PT100_vanh.enable50Hz(true);

  PT100_cua.begin(MAX31865_4WIRE);
  PT100_cua.enable50Hz(true);

  //lấy mẫu ban đầu để nhiệt độ lọc có giá trị
  for (uint8_t i = 0; i < 5; i++) {
    this->nhietDoLoc = LocCamBienBuong.updateEstimate(fLayNhietDoThucTe(PT100_buong)) + this->HeSoCalib;  //* lọc giá trị đọc
    this->NhietDoLoc_CUA = LocCamBienCua.updateEstimate(fLayNhietDoThucTe(PT100_cua));
  }

  triacBuong.init();
  triacCua.init();
  triacSau.init();
  triacQuat.init();
  /** Khởi tạo triac. */
  triac::configACDETPIN((gpio_num_t)ACDET_PIN);

  this->TatDieuKhienNhietDo();
  // khởi tạo task Bộ điều khiển dùng để tính toán giá trị kích cho triac
  xTaskCreate(TASK_TinhToanGiaTriDieuKhien, "task tinh toan gia tri ra", 13 * 1024, (void*)this, 2, &taskCalculateHandle);
}

//TODO: hàm callback của SOFTWARE TIMER
void HEATER::TASK_TinhToanGiaTriDieuKhien(void* _this) {
  HEATER* pClass = static_cast<HEATER*>(_this);
  TickType_t xLastWakeTime;
  xLastWakeTime = xTaskGetTickCount();
  while (1) {
    pClass->DieuKhienQuat();
    pClass->nhietDoLoc = pClass->LocCamBienBuong.updateEstimate(fLayNhietDoThucTe(pClass->PT100_buong)) + pClass->HeSoCalib;  //* lọc giá trị đọc
    pClass->NhietDoLoc_CUA = pClass->LocCamBienCua.updateEstimate(fLayNhietDoThucTe(pClass->PT100_cua));
    pClass->NhietDoLoc_VANH = pClass->LocCamBienVanh.updateEstimate(fLayNhietDoThucTe(pClass->PT100_vanh));

    pClass->GiamSatNhietDo();  //* giám sát nhiệt
    pClass->GiamSatNhietDo_CUA();

    pClass->ChayQuyTrinhGiaNhiet();  //* tính giá trị điều khiển và chạy quy trình thời gian bật, tắt và kiểm soát điện áp trên điện trở
    pClass->ChayQuyTrinhGiaNhiet_CUA();

    //* gán giá trị điều khiển cửa giải thuật ra
    pClass->triacQuat.SetTimeOverFlow(pClass->fanPower);

    pClass->u16ThoiGianGiaNhiet += 1;  //tăng thời gian chạy //! lưu ý đơn vị dựa vào chu kì enable task này MẶC ĐỊNH Phải là 1s
    pClass->u16ThoiGianGiaNhiet_CUA += 1;
    pClass->logDEBUG();

    //reset biến
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1000));
  }
}

void HEATER::CaiDatNhietDo(float fSetpointNhiet) {
  Serial.printf("Cai dat nhiet do: %f\n", fSetpointNhiet);
  triacBuong.turnOffPin();
  triacSau.turnOffPin();
  triacCua.turnOffPin();
  this->fSetpointNhiet = fSetpointNhiet;
  HeatUpTime = HeatUpTime_CUA = 0;
  CoolDownTime = 60;
  CoolDownTime_CUA = 5;
#ifdef TEST
  HeatUpTime = HeatUpTime_CUA = 0;
  CoolDownTime = 10;
  CoolDownTime_CUA = 10;
#endif
  this->u16ThoiGianGiaNhiet = this->u16ThoiGianGiaNhiet_CUA = 0;

  Laymau_CUA = Laymau = true;
  step = step_CUA = LamLanh;
}
void HEATER::TatDieuKhienNhietDo(bool RESET) {
  Serial.println("Tat dieu khien nhiet do");
  triacBuong.turnOffPin();
  triacSau.turnOffPin();
  triacCua.turnOffPin();
  triacQuat.TurnOffTriac();

  this->trangThaiDieuKhien = _TAT_DIEU_KHIEN_NHIET_DO;

  if (RESET == true) {
    giaTriGiaKhauI = 0;
  }

  HeatUpTime = HeatUpTime_CUA = 0;
  CoolDownTime = 60;
  CoolDownTime_CUA = 5;
  this->u16ThoiGianGiaNhiet = this->u16ThoiGianGiaNhiet_CUA = 0;
#ifdef TEST
  HeatUpTime = HeatUpTime_CUA = 0;
  CoolDownTime = 10;
  CoolDownTime_CUA = 10;
#endif
  Laymau_CUA = Laymau = true;
  step = step_CUA = LamLanh;
}
void HEATER::BatDieuKhienNhietDo(bool RESET) {
  Serial.println("Bat dieu khien nhiet do");
  triacQuat.TurnOnTriac();
  this->trangThaiDieuKhien = _BAT_DIEU_KHIEN_NHIET_DO;

  if (RESET == true) {
    giaTriGiaKhauI = 0;
  }

  HeatUpTime = HeatUpTime_CUA = 0;
  CoolDownTime = 60;
  CoolDownTime_CUA = 5;
  this->u16ThoiGianGiaNhiet = this->u16ThoiGianGiaNhiet_CUA = 0;
#ifdef TEST
  CoolDownTime = 10;
  CoolDownTime_CUA = 10;
#endif
  Laymau_CUA = Laymau = true;
  step = step_CUA = LamLanh;
}


// chạy quá trình tính toán giá trị - gia nhiệt và theo dõi độ tăng của nhiệt độ trong buồng
void HEATER::ChayQuyTrinhGiaNhiet() {
  static bool flagLoDo = false;
  float saiSo = this->fSetpointNhiet - this->nhietDoLoc;
  switch (step) {
    case eTinhToanGiaTriDieuKhien:
      GiaiThuat();
#ifdef TEST
      CoolDownTime = 10;
      CoolDownTime_CUA = 10;
#endif
      if (this->HeatUpTime > 0) {
#ifdef TEST_TRIAC
        this->triacBuong.turnOnPinAndDelayOff(123 + 1000);
#else
        this->triacBuong.turnOnPinAndDelayOff(this->HeatUpTime);
        this->HeatUpTime /= 1000;
#endif
      }
      if (this->HeatUpTimeMatSau > 0) {
#ifdef TEST_TRIAC
        this->triacSau.turnOnPinAndDelayOff(124 + 1000);
#else
        this->triacSau.turnOnPinAndDelayOff(this->HeatUpTimeMatSau);
        this->HeatUpTimeMatSau /= 1000;

#endif
      }

      this->u16ThoiGianGiaNhiet = 0;
      step = GiaNhiet;  //TODO chuyển trạng thái hàm
      // break; //TODO để sau khi tính toán xog thực hiện gia nhiệt ngay lập tức
    case GiaNhiet:
      if ((this->u16ThoiGianGiaNhiet >= this->HeatUpTime + 1) || (this->nhietDoLoc >= this->fSetpointNhiet)) {
        this->u16ThoiGianGiaNhiet = 0;
        if (saiSo <= 0.0f) {
          flagLoDo = true;  // mục đích sau khi hết lố bắt đầu theo dõi tính toán ngay
        }
        triacBuong.turnOffPin();
        triacSau.turnOffPin();
        step = LamLanh;  //TODO chuyển trạng thái hàm
      }
      break;
    case LamLanh:
      if ((u16ThoiGianGiaNhiet >= CoolDownTime) || (flagLoDo == true && saiSo > 0.0f)) {
        this->u16ThoiGianGiaNhiet = 0;
        flagLoDo = false;
        step = eTinhToanGiaTriDieuKhien;  //TODO chuyển trạng thái hàm
      }
      break;
  }
}


void HEATER::ChayQuyTrinhGiaNhiet_CUA() {

  static bool coGiaNhiet = false;  // false khi gia nhiệt từ sai số >0.5 mà bị lố
  static bool flagLoDo = false;
  float saiSo;
  // tắt gia nhiệt cửa khi tắt máy hoặc setpoint lớn hơn 60 tức vào chế độ tiệt trùng
  if (this->trangThaiDieuKhien == _TAT_DIEU_KHIEN_NHIET_DO || fSetpointNhiet > 60.0f) {
    step_CUA = LamLanh;
  }
  saiSo = this->fSetpointChoCua - this->NhietDoLoc_CUA;

  switch (step_CUA) {
    case eTinhToanGiaTriDieuKhien:
      this->HeatUpTime_CUA = 0;
      this->CoolDownTime_CUA = 10;
      if (saiSo > 0.5f) {
        coGiaNhiet = false;
        switch (this->TempState_CUA) {
          case NhietGiamNhanh:
          case NhietGiamCham:
          case KhongGiaNhiet:
          case NhietTangCham:
            {
              // tính nhiệt lượng
              float dentaT = saiSo - 0.5f;
              float nhietLuong = ((KHOI_LUONG_CUA * NHIET_DUNG_RIENG_CUA) / HIEU_SUAT_TRUYEN_NHIET_CUA
                                  + (KHOI_LUONG_KINH * NHIET_DUNG_RIENG_KINH) / HIEU_SUAT_TRUYEN_NHIET_KINH);
              float QSum = nhietLuong * dentaT;

              this->CoolDownTime_CUA = 60;

              // tính thời gian kích
              float VolTriac = 220;

              float Power = (VolTriac * VolTriac) / TONG_TRO_CUA;
              this->HeatUpTime_CUA = (uint32_t)(QSum * 1000 / Power);
            }
            break;

          default:
            break;
        }
      } else if (saiSo >= 0) {
        coGiaNhiet = true;
        switch (this->TempState_CUA) {
          case NhietGiamNhanh:
          case NhietGiamCham:
          case KhongGiaNhiet:
            {
              // tính nhiệt lượng
              float dentaT = saiSo;
              float nhietLuong = ((KHOI_LUONG_CUA * NHIET_DUNG_RIENG_CUA) / HIEU_SUAT_TRUYEN_NHIET_CUA
                                  + (KHOI_LUONG_KINH * NHIET_DUNG_RIENG_KINH) / HIEU_SUAT_TRUYEN_NHIET_KINH)
                                 / HIEU_SUAT_CHENH_LECH;
              float QSum = nhietLuong * dentaT;

              this->CoolDownTime_CUA = 60 - (0.5 - saiSo) * 60;
              if (this->CoolDownTime_CUA < 30) {
                this->CoolDownTime_CUA = 30;
              }

              // tính thời gian kích
              float VolTriac = 220;

              float Power = (VolTriac * VolTriac) / TONG_TRO_CUA;  //101.7207w
              this->HeatUpTime_CUA = (uint32_t)(QSum * 1000 / Power);
              break;
            }
          default:
            break;
        }
      } else {
        this->HeatUpTime_CUA = 0;
        this->CoolDownTime_CUA = 10;
      }

#ifdef TEST
      CoolDownTime = 10;
      CoolDownTime_CUA = 10;
#endif
      if (this->HeatUpTime_CUA > 0) {
#ifdef TEST_TRIAC
        this->triacCua.turnOnPinAndDelayOff(123);
#else
        this->triacCua.turnOnPinAndDelayOff(this->HeatUpTime_CUA);
        this->HeatUpTime_CUA /= 1000;
#endif
      }

      step_CUA = GiaNhiet;  //TODO chuyển trạng thái hàm
      // break; //TODO để sau khi tính toán xog thực hiện gia nhiệt ngay lập tức
    case GiaNhiet:
      if ((this->u16ThoiGianGiaNhiet_CUA > this->HeatUpTime_CUA + 1) || (NhietDoLoc_CUA >= fSetpointChoCua) || (coGiaNhiet == false && saiSo < 0.2f)) {
        this->u16ThoiGianGiaNhiet_CUA = 0;
        if (saiSo <= 0.0f) {
          flagLoDo = true;  // mục đích sau khi hết lố bắt đầu theo dõi tính toán ngay
        }
        triacCua.turnOffPin();
        step_CUA = LamLanh;  //TODO chuyển trạng thái hàm
      }
      break;
    case LamLanh:
      if ((u16ThoiGianGiaNhiet_CUA > CoolDownTime_CUA) || (flagLoDo == true && saiSo > 0.0f)) {
        this->u16ThoiGianGiaNhiet_CUA = 0;
        flagLoDo = false;
        step_CUA = eTinhToanGiaTriDieuKhien;  //TODO chuyển trạng thái hàm
      }
      break;
  }
}


//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM//
void HEATER::DieuKhienQuat() {
  // điều khiển quạt theo chu kì
  static uint32_t tg = 0;

  if (this->trangThaiDieuKhien == _BAT_DIEU_KHIEN_NHIET_DO) {
    tg++;
    if (tg <= ThoiGianBatQuat) {
      this->fanPower = 5000;
    } else if (ThoiGianBatQuat < tg && ThoiGianTatQuat > 0) {
      this->fanPower = 9600;
    }
    if (tg >= ThoiGianBatQuat + ThoiGianTatQuat) {
      tg = 0;
    }
  } else {
    tg = 0;
    this->fanPower = 9600;
  }
}

// giải thuật dựa vào độ dao động của nhiệt độ, và sai số nhiệt hiện tại để tính ra thời gian kích tương ứng với mức điện áp cố định trong buồng
void HEATER::GiaiThuat() {
  static int8_t soLanNhietTangChamTrog3chuKi = 0, f = 0, soLanNhietKhongTangChamTrog3chuKi = 0;
  static int8_t i = 0;
  // Hiện tại phần sai số nhiệt phân ra làm các mức sau
  //    + 10 độ, 5 độ , 2 độ ,1 độ ,0.5 độ , bé hơn 0.5 độ và bé hơn 0 độ

  // tắt gia máy hay tắt gia nhiệt thì cho chạy chế độ làm lạnh liên tục
  float saiSo = 0.0f;
  //* trạng thái bật gia nhiệt
  if (this->trangThaiDieuKhien == _BAT_DIEU_KHIEN_NHIET_DO) {
    saiSo = this->fSetpointNhiet - this->nhietDoLoc;
    // Giám sát nhiệt :
    // Biến this->TempState lấy từ hàm GiamSatNhiet()
    if (saiSo > 0.5f) {
      switch (this->TempState) {
        case NhietGiamNhanh:
        case NhietGiamCham:
        case KhongGiaNhiet:
          {
            // //! đảm bảo hiệu suất truyền nhiệt
            // float saiSoVachVaSetPoint = abs(NhietDoLoc_VACH - fSetpointNhiet);  // sai số giữa nhiệt vách và setpoint
            float hieuSuatThayDoi = 1.0f;
            // if (saiSoVachVaSetPoint <= 3.0f) {
            //   hieuSuatThayDoi = 1.0f + 0.5f * saiSoVachVaSetPoint;
            // } else {
            // hieuSuatThayDoi = 1.0f;
            // }

            // tính nhiệt lượng
            float dentaT = saiSo - 0.5f;
            float nhietLuong = ((KHOI_LUONG_BUONG * NHIET_DUNG_RIENG_BUONG) / HIEU_SUAT_TRUYEN_NHIET_BUONG
                                + (KHOI_LUONG_KHI * NHIET_DUNG_RIENG_KHI) / HIEU_SUAT_TRUYEN_NHIET_KHI)
                               / hieuSuatThayDoi;
            float QSum = nhietLuong * dentaT;

            this->CoolDownTime = 120;
            if (saiSo <= 2.0f) {
              this->CoolDownTime = 120 - (2 - saiSo) * 120;
            }

            if (this->CoolDownTime < 60) {
              this->CoolDownTime = 60;
            }


            // tính thời gian kích
            // float VolTriac = DIEN_AP_VAO_HIEU_DUNG * sqrt(1 - ((this->heatPower - 650) / 10000) + sin(2 * ((this->heatPower - 650) * PI / 10000)) / (2 * PI));
            float VolTriac = 220;

            float Power = (VolTriac * VolTriac) / TONG_TRO_BUONG;
            this->HeatUpTime = (uint32_t)(QSum * 1000 / Power) + i * 500;
            HeatUpTimeMatSau = HeatUpTime + offset;
            i++;
          }
          break;
        case NhietTangCham:
        case NhietTangNhanh:
          this->HeatUpTime = 0;
          this->CoolDownTime = 30;
          break;
      }
    }
    ////----------------------------------------------------0.5 - 0 độ------------------------------------------------//
    else if (saiSo >= 0.0f) {
      switch (this->TempState) {
        case NhietGiamNhanh:
        case NhietGiamCham:
        case KhongGiaNhiet:
          {
            // tính nhiệt lượng
            float dentaT = saiSo;
            float nhietLuong = ((KHOI_LUONG_BUONG * NHIET_DUNG_RIENG_BUONG) / HIEU_SUAT_TRUYEN_NHIET_BUONG
                                + (KHOI_LUONG_KHI * NHIET_DUNG_RIENG_KHI) / HIEU_SUAT_TRUYEN_NHIET_KHI)
                               / HIEU_SUAT_CHENH_LECH;  // khi thay đổi cơ cấu chỉ cần thay đổi số 0.15
            float QSum = nhietLuong * dentaT;

            this->CoolDownTime = 60 - (0.5 - saiSo) * 60;
            if (this->CoolDownTime < 30) {
              this->CoolDownTime = 30;
            }


            // tính thời gian kích
            // float VolTriac = DIEN_AP_VAO_HIEU_DUNG * sqrt(1 - ((this->heatPower - 650) / 10000) + sin(2 * ((this->heatPower - 650) * PI / 10000)) / (2 * PI));
            float VolTriac = 220;
            float Power = (VolTriac * VolTriac) / TONG_TRO_BUONG;  //101.7207w
            if (giaTriGiaKhauI < 0) {
              giaTriGiaKhauI = 0;
            }
            this->HeatUpTime = (uint32_t)(QSum * 1000 / Power) + giaTriGiaKhauI * 500;
            HeatUpTimeMatSau = HeatUpTime + offset;
            soLanNhietKhongTangChamTrog3chuKi += 1;
          }
          break;
        case NhietTangCham:
          soLanNhietTangChamTrog3chuKi += 1;
          this->HeatUpTime = 0;
          this->CoolDownTime = 30;
          break;
        case NhietTangNhanh:
          soLanNhietTangChamTrog3chuKi += 5;
          this->HeatUpTime = 0;
          this->CoolDownTime = 30;
          break;
      }

      // khởi tạo lại chu kì
      if (f == 0) {
        chuKi = 0;
        f = 1;
      }
      //* cộng khâu i
      if (chuKi >= 3) {
        if (soLanNhietTangChamTrog3chuKi <= 1 || soLanNhietKhongTangChamTrog3chuKi >= 1) {
          giaTriGiaKhauI += 2;
          soLanNhietTangChamTrog3chuKi = 0;
          soLanNhietKhongTangChamTrog3chuKi = 0;
        }
        chuKi = 0;
      }
      chuKi++;
      i = 0;


    }
    //----------------------------------------------------lố độ------------------------------------------------//
    else {
      //* khởi tạo lại chu kì
      if (f == 1) {
        chuKi = 0;
        f = 0;
      }
      //* trừ khâu i
      if (chuKi >= 8) {  // nếu lố quá 8 phút thì trừ vì thời gian giải 0.1-0.2 độ tầm 8-10 phút
        giaTriGiaKhauI -= 2;
        chuKi = 0;
      }
      chuKi++;

      this->HeatUpTime = 0;
      this->CoolDownTime = 60;

      i = 0;
    }
  }
  //* trạng thái tắt gia nhiệt
  else if (this->trangThaiDieuKhien == _TAT_DIEU_KHIEN_NHIET_DO) {
    this->HeatUpTime = 0;
    this->CoolDownTime = 10;
    this->fanPower = 0;
    step = LamLanh;
  }
}

// giám sát độ dao động cửa nhiệt độ sau khi gia nhiệt
void HEATER::GiamSatNhietDo() {
  //*------------------------------------------------- GIÁM XÁC ĐỘ ỔN ĐỊNH NHIỆT -------------------------------------------------
  static float pretemp;

  float Err = fabs(this->fSetpointNhiet - this->nhietDoLoc);
  if (Err <= 0.25f) {
    demOnDinh++;
    if (demOnDinh > 60) {
      demOnDinh = 60;
    }
  }

  //*---------------------------------------------- GIÁM SÁT TỐC ĐỘ THAY ĐỔI NHIỆT ĐỘ ----------------------------------------------
  //*sau khi gia nhiệt xog mới bắt đầu giám sát
  //* cộng dồn đến khi chuyển ngược về tính toán thì xác định trạng thái nhiệt sau khi chờ tỏa nhiệt
  if (step == LamLanh) {
    if (Laymau == true) {
      Laymau = false;
      pretemp = this->nhietDoLoc;
    }
  }
  //* kết thúc làm lạnh qay về tính toán thì tính ra trạng thái nhiệt sau khi theo dõi
  else if (step == eTinhToanGiaTriDieuKhien && this->CoolDownTime != 0) {

    // this->meanTemp = this->SumTemp / this->CoolDownTime;  // lấy giá trị trung bình
    // this->DeltaTemp = this->pretemp - this->meanTemp;
    float dentaT = pretemp - this->nhietDoLoc;

    if (dentaT > 0.008) {
      this->TempState = NhietGiamCham;
    } else if (dentaT > 0.05) {
      this->TempState = NhietGiamNhanh;
    } else if (dentaT < -0.05) {
      this->TempState = NhietTangNhanh;
    } else if (dentaT < -0.008) {
      this->TempState = NhietTangCham;
    } else {
      this->TempState = KhongGiaNhiet;
    }

    Laymau = true;
    // this->SumTemp = 0;
    // this->pretemp = this->meanTemp;
  }
}
//* giám sát nhiệt độ cửa
void HEATER::GiamSatNhietDo_CUA() {
  static float preTemp;
  //*---------------------------------------------- GIÁM SÁT TỐC ĐỘ THAY ĐỔI NHIỆT ĐỘ ----------------------------------------------
  //*sau khi gia nhiệt xog mới bắt đầu giám sát
  //* cộng dồn đến khi chuyển ngược về tính toán thì xác định trạng thái nhiệt sau khi chờ tỏa nhiệt
  if (step_CUA == LamLanh) {
    if (Laymau_CUA == true) {
      Laymau_CUA = false;
      preTemp = this->NhietDoLoc_CUA;
    }
  }
  //* kết thúc làm lạnh qay về tính toán thì tính ra trạng thái nhiệt sau khi theo dõi
  else if (step_CUA == eTinhToanGiaTriDieuKhien && this->CoolDownTime_CUA != 0) {

    // this->meanTemp = this->SumTemp / this->CoolDownTime;  // lấy giá trị trung bình
    // this->DeltaTemp = this->pretemp - this->meanTemp;
    float dentaT = preTemp - this->NhietDoLoc_CUA;

    if (dentaT > 0.008) {
      this->TempState_CUA = NhietGiamCham;
    } else if (dentaT > 0.05) {
      this->TempState_CUA = NhietGiamNhanh;
    } else if (dentaT < -0.05) {
      this->TempState_CUA = NhietTangNhanh;
    } else if (dentaT < -0.008) {
      this->TempState_CUA = NhietTangCham;
    } else {
      this->TempState_CUA = KhongGiaNhiet;
    }

    Laymau_CUA = true;
  }
}

void HEATER::logDEBUG() {
  switch (TempState) {
    case NhietGiamCham:
      NHIET_SerialPrintf(CoChoPhepLogDebug, "GiamCham-");
      break;
    case NhietGiamNhanh:
      NHIET_SerialPrintf(CoChoPhepLogDebug, "GiamNhanh-");
      break;
    case NhietTangNhanh:
      NHIET_SerialPrintf(CoChoPhepLogDebug, "TangNhanh-");
      break;
    case NhietTangCham:
      NHIET_SerialPrintf(CoChoPhepLogDebug, "TangCham-");
      break;
    case KhongGiaNhiet:
      NHIET_SerialPrintf(CoChoPhepLogDebug, "OnDinh-");
      break;
    case KhongXacDinh:
      NHIET_SerialPrintf(CoChoPhepLogDebug, "X-");
      break;
  }
  if (this->trangThaiDieuKhien == _BAT_DIEU_KHIEN_NHIET_DO) {
    this->TempState = KhongXacDinh;  // log debug xong thì reset giá trị
  }
  NHIET_SerialPrintf(CoChoPhepLogDebug, "chuKi(%u)-", chuKi);
  NHIET_SerialPrintf(CoChoPhepLogDebug, "giaTriGiaKhauI( %d )-", this->giaTriGiaKhauI);
  NHIET_SerialPrintf(CoChoPhepLogDebug, "TGGiaNhiet( %u )-", this->HeatUpTime);
  NHIET_SerialPrintf(CoChoPhepLogDebug, "TGLamLanh(%u)-", this->CoolDownTime);
  NHIET_SerialPrintf(CoChoPhepLogDebug, "TGDaChay(%u)-", this->u16ThoiGianGiaNhiet);
  NHIET_SerialPrintf(CoChoPhepLogDebug, "TGGiaNhietSau( %u )-", this->HeatUpTimeMatSau);
  NHIET_SerialPrintf(CoChoPhepLogDebug, "offset( %u )-", this->offset);
  NHIET_SerialPrintf(CoChoPhepLogDebug, "Fan(%lu)-", this->fanPower);
  NHIET_SerialPrintf(CoChoPhepLogDebug, "SP(%0.2f)-", this->fSetpointNhiet);
  NHIET_SerialPrintf(CoChoPhepLogDebug, "Nhiet( %0.3f )", this->nhietDoLoc);

  switch (TempState_CUA) {
    case NhietGiamCham:
      NHIET_SerialPrintf(CoChoPhepLogDebug, "-GiamCham-");
      break;
    case NhietGiamNhanh:
      NHIET_SerialPrintf(CoChoPhepLogDebug, "-GiamNhanh-");
      break;
    case NhietTangNhanh:
      NHIET_SerialPrintf(CoChoPhepLogDebug, "-TangNhanh-");
      break;
    case NhietTangCham:
      NHIET_SerialPrintf(CoChoPhepLogDebug, "-TangCham-");
      break;
    case KhongGiaNhiet:
      NHIET_SerialPrintf(CoChoPhepLogDebug, "-OnDinh-");
      break;
    case KhongXacDinh:
      NHIET_SerialPrintf(CoChoPhepLogDebug, "-X-");
      break;
  }
  if (this->trangThaiDieuKhien == _BAT_DIEU_KHIEN_NHIET_DO) {
    this->TempState_CUA = KhongXacDinh;  // log debug xong thì reset giá trị
  }
  NHIET_SerialPrintf(CoChoPhepLogDebug, "TGGiaNhietCUA( %u )-", this->HeatUpTime_CUA);
  NHIET_SerialPrintf(CoChoPhepLogDebug, "TGLamLanhCUA(%u)-", this->CoolDownTime_CUA);
  NHIET_SerialPrintf(CoChoPhepLogDebug, "TGDaChayCUA(%u)-", this->u16ThoiGianGiaNhiet_CUA);
  NHIET_SerialPrintf(CoChoPhepLogDebug, "SPCUA(%0.2f)-", this->fSetpointChoCua);
  NHIET_SerialPrintf(CoChoPhepLogDebug, "NhietCUA( %0.3f )\n", this->NhietDoLoc_CUA);
}

static float fLayNhietDoThucTe(Adafruit_MAX31865& xPt100) {
  float fNhietDo = 999;
  uint8_t u8Try = 5;
  while (u8Try--) {
    fNhietDo = xPt100.temperature(MAX31865_DIEN_TRO_CAM_BIEN, MAX31865_DIEN_TRO_THAM_CHIEU);
    if (abs(fNhietDo) < MAX31865_NHIET_DO_TOI_DA) {
      break;
    }
    fNhietDo = 999;
    xPt100.begin(MAX31865_4WIRE);
    xPt100.enable50Hz(true);
    delay(10);
  }
  return fNhietDo;
}
