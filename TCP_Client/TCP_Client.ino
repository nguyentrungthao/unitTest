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
#include <EthernetESP32.h>
#include <Adafruit_MAX31865.h>
#include "driver/spi_slave.h"
#include "driver/spi_master.h"

SPIClass PT100_SPI(HSPI);

// #include <Ethernet.h>

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
// The value of the Rref resistor. Use 430.0 for PT100 and 4300.0 for PT1000
#define RREF 390.0
// The 'nominal' 0-degrees-C resistance of the sensor
// 100.0 for PT100, 1000.0 for PT1000
#define RNOMINAL 100.0
// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192, 168, 137, 6);
uint16_t port = 12345;
// Enter the IP address of the server you're connecting to:
IPAddress server(192, 168, 137, 5);
// Use software SPI: CS, DI, DO, CLK
Adafruit_MAX31865 thermo = Adafruit_MAX31865(39, &PT100_SPI);

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 23 is default for telnet;
// if you're using Processing's ChatServer, use port 10002):
EthernetClient client;

void setup() {
  SPI.begin(ETH_SCK_PIN, ETH_MISO_PIN, ETH_MOSI_PIN, ETH_CS_PIN);
  PT100_SPI.begin(PT100_SCK_PIN, PT100_MISO_PIN, PT100_MOSI_PIN, PT100_CS1_PIN);
  // You can use Ethernet.init(pin) to configure the CS pin
  //Ethernet.init(10);  // Most Arduino shields
  //Ethernet.init(5);   // MKR ETH Shield
  //Ethernet.init(0);   // Teensy 2.0
  //Ethernet.init(20);  // Teensy++ 2.0
  //Ethernet.init(15);  // ESP8266 with Adafruit FeatherWing Ethernet
  Ethernet.init(ETH_CS_PIN);     // ESP32 with Adafruit FeatherWing Ethernet
  thermo.begin(MAX31865_4WIRE);  // set to 2WIRE or 4WIRE as necessary

  // start the Ethernet connection:
  Ethernet.begin(mac, ip);

  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }

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
}

void loop() {
  Serial.print("Temperature = ");
  Serial.println(thermo.temperature(RNOMINAL, RREF));
  // // if there are incoming bytes available
  // // from the server, read them and print them:
  if (client.available()) {
    String str = client.readString();
  }

  // as long as there are bytes in the serial queue,
  // read them and send them out the socket if it's open:
  while (Serial.available() > 0) {
    char inChar = Serial.read();
    if (client.connected()) {
      client.print(inChar);
    }
  }

  // if the server's disconnected, stop the client:
  if (!client.connected()) {
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();
    // do nothing:
    // while (true) {
    //   delay(1);
    // }
  }
  delay(10);
}
