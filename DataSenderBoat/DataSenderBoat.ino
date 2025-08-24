#include <SPI.h>
#include <LoRa.h>
#include <TinyGPS++.h>

// === PIN DEFINITIONS ===
#define WATER_SENSOR_PIN 25
#define GPS_RX_PIN 21
#define GPS_TX_PIN 22
#define LORA_CS 5
#define LORA_RST 14
#define LORA_DIO0 2

// === GPS SETUP ===
HardwareSerial SerialGPS(2);
TinyGPSPlus gps;

void setup() {
  Serial.begin(115200);
  SerialGPS.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
  delay(2000);

  pinMode(WATER_SENSOR_PIN, INPUT_PULLUP);

  // LoRa setup
  LoRa.setPins(LORA_CS, LORA_RST, LORA_DIO0);
  if (!LoRa.begin(433E6)) {
    Serial.println("LoRa init failed. Check wiring/antenna!");
    while (1);
  }

  // === Long range LoRa settings ===
  LoRa.setTxPower(20, PA_OUTPUT_PA_BOOST_PIN); // Max power
  LoRa.setSpreadingFactor(12);                 // Max spreading factor
  LoRa.setSignalBandwidth(62.5E3);             // Narrow bandwidth
  LoRa.setCodingRate4(8);                      // Strongest error correction

  Serial.println("Sender ready (long-range mode).");
}

void loop() {
  // Read GPS data
  while (SerialGPS.available()) {
    gps.encode(SerialGPS.read());
  }

  int water = digitalRead(WATER_SENSOR_PIN);

  if (water == HIGH && gps.location.isValid()) {
    String message = "ALERT: Boat sinking detected! Location: ";
    message += gps.location.lat();
    message += ", ";
    message += gps.location.lng();

    Serial.println("📡 Sending: " + message);
    LoRa.beginPacket();
    LoRa.print(message);
    LoRa.endPacket();

    delay(10000); // Send every 10 sec while water detected
  } else {
    Serial.println("⏳ Waiting for water or GPS fix...");
    delay(1000);
  }
}
