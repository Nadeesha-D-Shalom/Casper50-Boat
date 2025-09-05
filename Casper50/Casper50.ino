#include <SPI.h>
#include <LoRa.h>
#include <TinyGPS++.h>

// ====== PIN DEFINITIONS ======
#define LORA_CS   5
#define LORA_RST  14
#define LORA_DIO0 2

#define GPS_RX_PIN 21   // GPS TX → ESP32 RX2 (GPIO21)
#define GPS_TX_PIN 22   // GPS RX → ESP32 TX2 (GPIO22)

// Motor driver (e.g., L298N/L293D style)
#define ENA 25   // PWM Channel A
#define IN1 26
#define IN2 27
#define IN3 33
#define IN4 32
#define ENB 15   // PWM Channel B

// ====== LEDC (ESP32 PWM) SETTINGS ======
const int PWM_FREQ     = 20000; // 20 kHz (quiet/efficient on ESP32)
const int PWM_RES_BITS = 8;     // 0..255 duty
const int PWM_CH_A     = 0;     // LEDC channels (0..15)
const int PWM_CH_B     = 1;

// ====== MOTOR SPEEDS (0..255) ======
int SPEED_A = 180;
int SPEED_B = 180;

// ====== LoRa / GPS VARS ======
String receivedMessage = "";
int rssi = 0;
float snr = 0;

HardwareSerial SerialGPS(2);
TinyGPSPlus gps;

unsigned long lastGPSPrint = 0;
const unsigned long gpsInterval = 5000; // print local GPS every 5 sec

// ---------- MOTOR HELPERS ----------
void motorSetSpeed(int speedA, int speedB) {
  if (speedA < 0) speedA = 0; if (speedA > 255) speedA = 255;
  if (speedB < 0) speedB = 0; if (speedB > 255) speedB = 255;
  ledcWrite(PWM_CH_A, speedA);
  ledcWrite(PWM_CH_B, speedB);
}

void motorForward() {
  // A forward
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  // B forward
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  motorSetSpeed(SPEED_A, SPEED_B);
}

void motorBackward() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  motorSetSpeed(SPEED_A, SPEED_B);
}

void motorLeft() {
  // Left turn: A backward, B forward
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  motorSetSpeed(SPEED_A, SPEED_B);
}

void motorRight() {
  // Right turn: A forward, B backward
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  motorSetSpeed(SPEED_A, SPEED_B);
}

void motorStop() {
  // Brake both sides
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  motorSetSpeed(0, 0);
}

// ---------- SETUP ----------
void setup() {
  Serial.begin(115200);
  SerialGPS.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
  delay(1000);

  // Motor pins
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);

  // PWM attach
  // ledcSetup(PWM_CH_A, PWM_FREQ, PWM_RES_BITS);
  // ledcSetup(PWM_CH_B, PWM_FREQ, PWM_RES_BITS);
  // ledcAttachPin(ENA, PWM_CH_A);
  // ledcAttachPin(ENB, PWM_CH_B);

  // Start motors forward (non-blocking)
  motorForward();

  // LoRa setup
  LoRa.setPins(LORA_CS, LORA_RST, LORA_DIO0);
  if (!LoRa.begin(433E6)) {
    Serial.println("LoRa init failed. Check wiring/antenna!");
    while (1) { delay(1000); }
  }

  // Long-range LoRa settings (must match sender)
  LoRa.setTxPower(20, PA_OUTPUT_PA_BOOST_PIN);
  LoRa.setSpreadingFactor(12);
  LoRa.setSignalBandwidth(62.5E3);
  LoRa.setCodingRate4(8);
  LoRa.setSyncWord(0x12);

  Serial.println("=================================");
  Serial.println("Casper50 Receiver + Motors + GPS");
  Serial.println("Frequency: 433 MHz | SF:12 | BW:62.5kHz | CR:4/8");
  Serial.println("Motors running forward; GPS + LoRa non-blocking");
  Serial.println("=================================");
}

// ---------- LOOP ----------
void loop() {
  // 1) Feed GPS parser (non-blocking)
  while (SerialGPS.available()) {
    gps.encode(SerialGPS.read());
  }

  // 2) Print this receiver's own GPS every 5 seconds
  unsigned long now = millis();
  if (now - lastGPSPrint >= gpsInterval) {
    lastGPSPrint = now;
    if (gps.location.isValid()) {
      double lat = gps.location.lat();
      double lng = gps.location.lng();
      Serial.println();
      Serial.println("[Receiver GPS]");
      Serial.print("Lat: "); Serial.println(lat, 6);
      Serial.print("Lng: "); Serial.println(lng, 6);
      Serial.print("Google Maps: https://maps.google.com/?q=");
      Serial.print(lat, 6); Serial.print(","); Serial.println(lng, 6);
    } else {
      Serial.println();
      Serial.println("[Receiver GPS] Waiting for fix...");
    }
  }

  // 3) LoRa packet check (non-blocking)
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    receivedMessage = "";
    while (LoRa.available()) {
      receivedMessage += (char)LoRa.read();
    }

    rssi = LoRa.packetRssi();
    snr  = LoRa.packetSnr();

    Serial.println();
    Serial.println("=== EMERGENCY SIGNAL RECEIVED ===");
    Serial.print("Uptime: "); Serial.print(millis()/1000); Serial.println(" s");
    Serial.print("Message: "); Serial.println(receivedMessage);
    Serial.print("RSSI: "); Serial.print(rssi); Serial.println(" dBm");
    Serial.print("SNR: ");  Serial.print(snr);  Serial.println(" dB");

    // Parse "Location: <lat>, <lng>"
    int tag = receivedMessage.indexOf("Location:");
    if (tag != -1) {
      int latStart = tag + 10;
      int commaPos = receivedMessage.indexOf(",", latStart);
      if (commaPos != -1) {
        String latitude  = receivedMessage.substring(latStart, commaPos);
        String longitude = receivedMessage.substring(commaPos + 2);

        Serial.println("[Boat GPS from LoRa]");
        Serial.print("Latitude: ");  Serial.println(latitude);
        Serial.print("Longitude: "); Serial.println(longitude);
        Serial.print("Google Maps: https://maps.google.com/?q=");
        Serial.print(latitude); Serial.print(","); Serial.println(longitude);
      }
    }
    Serial.println("=================================");
  }

}
