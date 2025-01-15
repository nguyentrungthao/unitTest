/*
 Telnet client

 This sketch connects to a telnet server (http://www.google.com)
 using an Arduino WIZnet Ethernet shield.  You'll need a telnet server
 to test this with.
 Processing's ChatServer example (part of the Network library) works well,
 running on port 10002. It can be found as part of the examples
 in the Processing application, available at
 https://processing.org/

 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13

 created 14 Sep 2010
 modified 9 Apr 2012
 by Tom Igoe
 */

#include <SPI.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <EthernetESP32.h>
#include <Adafruit_MAX31865.h>
#include "driver/spi_slave.h"
#include "driver/spi_master.h"
#include "AnhLABV01HardWare.h"
SPIClass PT100_SPI(HSPI);


#define PT100_MOSI_PIN 42
#define PT100_MISO_PIN 41
#define PT100_SCK_PIN 40
#define PT100_CS1_PIN 39
#define PT100_CS2_PIN 38
#define PT100_CS3_PIN 37

#define ETH_MOSI_PIN 16
#define ETH_MISO_PIN 17
#define ETH_SCK_PIN 18
#define ETH_INT_PIN 15
#define ETH_CS_PIN 8
#define RREF 390.0
#define RNOMINAL 100.0

#define ETH_GET_DATA (1 << 0)
#define ETH_SEND_DATA (1 << 1)
Adafruit_MAX31865 thermo = Adafruit_MAX31865(39, &PT100_SPI);

EthernetClient client;
EventGroupHandle_t xEventGroupReceiceETH;
String stringToETH;

static byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
static IPAddress ip(192, 168, 137, 6);
static uint16_t port = 12345;
// Enter the IP address of the server you're connecting to:
static IPAddress server(192, 168, 137, 5);

void IRAM_ATTR ethISR(void *arg) {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xEventGroupSetBitsFromISR(xEventGroupReceiceETH, ETH_GET_DATA, &xHigherPriorityTaskWoken);
}

void taskETH(void *ptr) {

  SPI.begin(ETH_SCK_PIN, ETH_MISO_PIN, ETH_MOSI_PIN, ETH_CS_PIN);
  Ethernet.init(ETH_CS_PIN);  // ESP32 with Adafruit FeatherWing Ethernet
  // start the Ethernet connection:
  Ethernet.begin(mac, ip);
  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    // while (true) {
    //   delay(1);  // do nothing, no point running without Ethernet hardware
    // }
  }
  while (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
    delay(500);
  }

  // give the Ethernet shield a second to initialize:
  delay(1000);
  Serial.println("connecting...");

  // if you get a connection, report back via serial:
  if (client.connect(server, port)) {
    Serial.println("connected");
  } else {
    // if you didn't get a connection to the server:
    Serial.println("connection failed");
  }

  attachInterruptArg(ETH_INT_PIN, ethISR, NULL, FALLING);

  EventBits_t bit = 0;

  while (1) {
    bit = xEventGroupWaitBits(xEventGroupReceiceETH, ETH_GET_DATA | ETH_SEND_DATA, pdTRUE, pdFALSE, portMAX_DELAY);
    if ((bit & ETH_GET_DATA) != 0) {
      // if there are incoming bytes available
      // from the server, read them and print them:
      if (client.available()) {
        char c = client.read();
        Serial.print(c);
      }
    }
    if ((bit & ETH_SEND_DATA) != 0) {
      if (client.connected()) {
        client.print(stringToETH);
      }
    }

    delay(10);
  }
}
void taskReadSensor(void *ptr) {
  PT100_SPI.begin(PT100_SCK_PIN, PT100_MISO_PIN, PT100_MOSI_PIN, PT100_CS1_PIN);
  thermo.begin(MAX31865_4WIRE);  // set to 2WIRE or 4WIRE as necessary
  while (1) {
    Serial.print("Temperature = ");
    Serial.println(thermo.temperature(RNOMINAL, RREF));
    delay(1000);
  }
}

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }

  xEventGroupReceiceETH = xEventGroupCreate();
  xTaskCreatePinnedToCore(taskETH, "eth", 4048, NULL, 2, NULL, 1);
  xTaskCreatePinnedToCore(taskReadSensor, "sensor", 4048, NULL, 1, NULL, 0);
}

void loop() {

  // as long as there are bytes in the serial queue,
  // read them and send them out the socket if it's open:
  while (Serial.available() > 0) {
    stringToETH = Serial.readString();
    xEventGroupSetBits(xEventGroupReceiceETH, ETH_SEND_DATA);
  }

  // if the server's disconnected, stop the client:
  if (!client.connected()) {
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();
    // if you get a connection, report back via serial:
    if (client.connect(server, port)) {
      Serial.println("connected");
    } else {
      // if you didn't get a connection to the server:
      Serial.println("connection failed");
      delay(1000);
    }
  }

  delay(1);
}
