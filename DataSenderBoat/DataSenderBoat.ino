#include <SPI.h>
#include <LoRa.h>

#define WATER_SENSOR_PIN 25

void setup() {
  Serial.begin(115200);
  delay(2000);
  setupWaterSensor();
  setupLoRa();
}

void loop() {
  checkWaterLevel();
  delay(1000);
}

void setupWaterSensor() {
  pinMode(WATER_SENSOR_PIN, INPUT_PULLUP);
  Serial.println("Water sensor initialized.");
}

void setupLoRa() {
  Serial.println("Starting LoRa Transmitter...");
  LoRa.setPins(5, 14, 2);
  if (!LoRa.begin(433E6)) {
    Serial.println("LoRa init failed. Check wiring.");
    while (true);
  }
  Serial.println("LoRa init success");
}

void checkWaterLevel() {
  int sensorValue = digitalRead(WATER_SENSOR_PIN);
  if (sensorValue == HIGH) {
    Serial.println("Water detected! Sending LoRa alert...");
    LoRa.beginPacket();
    LoRa.print("ALERT: Boat sinking detected!");
    LoRa.endPacket();
    delay(10000);
  } else {
    Serial.println("No water.");
  }
}
