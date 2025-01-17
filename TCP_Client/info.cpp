#include "Arduino.h"
#include "FS.h"
#include "SPIFFS.h"

void vPrintInfo() {

  // Tìm tất cả các phân vùng
  esp_partition_iterator_t iter = esp_partition_find(ESP_PARTITION_TYPE_ANY, ESP_PARTITION_SUBTYPE_ANY, NULL);

  if (iter == NULL) {
    Serial.println("Không tìm thấy phân vùng nào.");
    return;
  }

  // Duyệt qua các phân vùng tìm được
  while (iter != NULL) {
    const esp_partition_t* partition = esp_partition_get(iter);

    // In thông tin về phân vùng
    Serial.print("Partition Label: ");
    Serial.println(partition->label);
    Serial.print("Partition Type: ");
    Serial.println(partition->type);
    Serial.print("Partition Subtype: ");
    Serial.println(partition->subtype);
    Serial.print("Start Address: ");
    Serial.println(partition->address, HEX);
    Serial.print("Size: ");
    Serial.println(partition->size);

    // Chuyển sang phân vùng tiếp theo
    iter = esp_partition_next(iter);
  }

  // Giải phóng iterator sau khi sử dụng
  esp_partition_iterator_release(iter);

  // In ra các thông tin liên quan đến ESP32
  Serial.println("ESP32 Flash Information:");
  Serial.print("Flash Size: ");
  Serial.println(ESP.getFlashChipSize());
  Serial.print("Flash Speed: ");
  Serial.println(ESP.getFlashChipSpeed());
  Serial.print("Flash Mode: ");
  Serial.println(ESP.getFlashChipMode());
}