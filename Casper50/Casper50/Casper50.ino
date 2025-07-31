#include <SPI.h>
#include <LoRa.h>

void setup() {
  Serial.begin(115200);
  delay(2000);
  setupLoRa();
}

void loop() {
  receiveLoRaPacket();
}

void setupLoRa() {
  Serial.println("Starting LoRa Receiver...");
  LoRa.setPins(5, 14, 2);
  if (!LoRa.begin(433E6)) {
    Serial.println("LoRa init failed. Check wiring.");
    while (true);
  }
  Serial.println("LoRa Receiver is ready.");
}

void receiveLoRaPacket() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    Serial.print("Received packet: ");
    while (LoRa.available()) {
      String message = LoRa.readString();
      Serial.print(message);
    }
    Serial.println();
  }
}
