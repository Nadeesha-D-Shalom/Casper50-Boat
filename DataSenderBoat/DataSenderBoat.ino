#include <SPI.h>
#include <LoRa.h>
#include <TinyGPS++.h>

#define WATER_SENSOR_PIN 25
#define GPS_RX_PIN 21
#define GPS_TX_PIN 22

HardwareSerial SerialGPS(2);
TinyGPSPlus gps;

void setup() {
  Serial.begin(115200);
  SerialGPS.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
  delay(2000);

  pinMode(WATER_SENSOR_PIN, INPUT_PULLUP);

  LoRa.setPins(5, 14, 2);
  if (!LoRa.begin(433E6)) {
    Serial.println("LoRa init failed.");
    while (1);
  }

  LoRa.setTxPower(20, PA_OUTPUT_PA_BOOST_PIN);
  LoRa.setSpreadingFactor(12);
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(5);

  Serial.println("Sender ready.");
}

void loop() {
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
    delay(10000);
  } else {
    Serial.println(" Waiting for water or GPS fix...");
  }

  delay(1000);
}
