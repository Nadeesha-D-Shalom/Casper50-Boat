#include <SPI.h>
#include <LoRa.h>

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("Starting LoRa Receiver...");
  LoRa.setPins(5, 14, 2); // CS=D5, RST=D14, DIO0=D2

  if (!LoRa.begin(433E6)) {
    Serial.println("LoRa init failed. Check wiring.");
    while (true);
  }

  LoRa.setTxPower(20, PA_OUTPUT_PA_BOOST_PIN);
  LoRa.setSpreadingFactor(12);
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(5);

  Serial.println(" LoRa Receiver is ready.");
}

void loop() {
  int packetSize = LoRa.parsePacket();

  if (packetSize) {
    String received = "";
    while (LoRa.available()) {
      received += (char)LoRa.read();
    }

    Serial.println(" Received: " + received);
    Serial.print(" RSSI: ");
    Serial.print(LoRa.packetRssi());
    Serial.println(" dBm");
  } else {
    Serial.println(" Waiting for packet...");
    delay(1000);
  }
}
