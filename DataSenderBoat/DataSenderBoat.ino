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

// === SIGNAL CONFIRMATION ===
bool firstSignalSent = false;
unsigned long firstSignalTime = 0;
bool confirmationSent = false;

// === GPS SETUP ===
HardwareSerial SerialGPS(2);
TinyGPSPlus gps;

// === GPS AVERAGING ===
double avgLat = 0.0;
double avgLng = 0.0;
int avgCount = 0;
const int AVG_SAMPLES = 10;  // Average over 10 samples for better accuracy
bool avgComplete = false;
unsigned long lastAvgReset = 0;

void setup() {
  Serial.begin(115200);
  SerialGPS.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
  delay(2000);

  pinMode(WATER_SENSOR_PIN, INPUT_PULLUP);

  // LoRa setup
  LoRa.setPins(LORA_CS, LORA_RST, LORA_DIO0);
  if (!LoRa.begin(433E6)) {
    Serial.println("LoRa init failed. Check wiring/antenna!");
    while (1)
      ;
  }

  // === Long range LoRa settings ===
  LoRa.setTxPower(20, PA_OUTPUT_PA_BOOST_PIN);  // Max power
  LoRa.setSpreadingFactor(12);                  // Max spreading factor
  LoRa.setSignalBandwidth(62.5E3);              // Narrow bandwidth
  LoRa.setCodingRate4(8);                       // Strongest error correction
  LoRa.setSyncWord(0x12);                       // Must match receiver

  Serial.println("=================================");
  Serial.println("LoRa Sender with GPS (Long-range)");
  Serial.println("Water sensor on D25");
  Serial.println("GPS: TX->D22, RX->D21");
  Serial.println("GPS Averaging: " + String(AVG_SAMPLES) + " samples");
  Serial.println("=================================\n");
}

void loop() {
  // Read GPS data
  while (SerialGPS.available()) {
    gps.encode(SerialGPS.read());
  }

  // Read water sensor
  int water = digitalRead(WATER_SENSOR_PIN);

  // Display GPS status every 2 seconds
  static unsigned long lastStatusUpdate = 0;
  if (millis() - lastStatusUpdate > 2000) {
    displayGPSStatus();
    lastStatusUpdate = millis();
  }

  // Reset averaging if water sensor goes dry
  if (water == LOW) {
    if (avgCount > 0 || firstSignalSent) {
      Serial.println(" Water sensor dry - resetting GPS averaging and signals");
      avgLat = 0.0;
      avgLng = 0.0;
      avgCount = 0;
      avgComplete = false;
      firstSignalSent = false;
      confirmationSent = false;
    }
    delay(1000);
    return;
  }

  // Send alert if water detected and GPS is valid
  if (water == HIGH && gps.location.isValid()) {
    // Check GPS accuracy before processing
    bool gpsAccuracyGood = (gps.hdop.isValid() && gps.hdop.hdop() < 5.0 && gps.satellites.value() >= 5) ||
                           (gps.satellites.value() >= 7);
    
    if (!gpsAccuracyGood) {
      Serial.println("⚠  Water detected but GPS accuracy poor. Waiting for better fix...");
      Serial.println("   HDOP: " + String(gps.hdop.hdop(), 1) + ", Sats: " + String(gps.satellites.value()));
      delay(1000);
      return;
    }

    // Perform GPS averaging for stationary accuracy
    if (!avgComplete) {
      if (avgCount < AVG_SAMPLES) {
        avgLat += gps.location.lat();
        avgLng += gps.location.lng();
        avgCount++;
        
        Serial.println(" Averaging GPS position... " + String(avgCount) + "/" + String(AVG_SAMPLES));
        Serial.println("   Current: " + String(gps.location.lat(), 6) + ", " + String(gps.location.lng(), 6));
        
        delay(1000);  // Sample every second
        return;
      } else {
        avgComplete = true;
        Serial.println(" GPS averaging complete!");
      }
    }
    
    // Calculate averaged position
    double sendLat = avgLat / AVG_SAMPLES;
    double sendLng = avgLng / AVG_SAMPLES;

    // Calculate position drift from average
    double driftLat = abs(gps.location.lat() - sendLat) * 111000;  // meters
    double driftLng = abs(gps.location.lng() - sendLng) * 111000 * cos(radians(sendLat));  // meters
    double drift = sqrt(driftLat * driftLat + driftLng * driftLng);

    // FIRST SIGNAL
    if (!firstSignalSent) {
      String message = "ALERT: Boat sinking detected! Location: ";
      message += String(sendLat, 6);
      message += ", ";
      message += String(sendLng, 6);

      Serial.println("\n === FIRST ALERT === ");
      Serial.println("Message: " + message);
      Serial.println("Averaged from " + String(AVG_SAMPLES) + " samples");
      Serial.println("GPS Quality: GOOD (" + String(gps.satellites.value()) + " sats, HDOP: " + String(gps.hdop.hdop(), 1) + ")");
      Serial.println("Current drift from average: " + String(drift, 1) + "m");
      
      LoRa.beginPacket();
      LoRa.print(message);
      LoRa.endPacket();
      
      firstSignalSent = true;
      firstSignalTime = millis();
      Serial.println(" First signal sent! Waiting 10s for confirmation...");
      Serial.println("=====================================\n");
      return;
    }

    // CONFIRMATION SIGNAL - After 10 seconds
    if (firstSignalSent && !confirmationSent && (millis() - firstSignalTime >= 10000)) {
      String message = "ALERT: Boat sinking detected! Location: ";
      message += String(sendLat, 6);
      message += ", ";
      message += String(sendLng, 6);

      Serial.println("\n === CONFIRMATION === ");
      Serial.println("Message: " + message);
      Serial.println("Averaged from " + String(AVG_SAMPLES) + " samples");
      Serial.println("GPS Quality: GOOD (" + String(gps.satellites.value()) + " sats, HDOP: " + String(gps.hdop.hdop(), 1) + ")");
      Serial.println("Current drift from average: " + String(drift, 1) + "m");
      Serial.println("CASPER 50 will respond!");
      
      LoRa.beginPacket();
      LoRa.print(message);
      LoRa.endPacket();
      
      confirmationSent = true;
      Serial.println(" Confirmation sent!");
      Serial.println("=====================================\n");
      return;
    }

    // SUBSEQUENT SIGNALS - After confirmation
    if (confirmationSent) {
      String message = "ALERT: Boat sinking detected! Location: ";
      message += String(sendLat, 6);
      message += ", ";
      message += String(sendLng, 6);

      Serial.println("\n === UPDATE SIGNAL === ");
      Serial.println("Message: " + message);
      Serial.println("Averaged from " + String(AVG_SAMPLES) + " samples");
      Serial.println("GPS Quality: GOOD (" + String(gps.satellites.value()) + " sats, HDOP: " + String(gps.hdop.hdop(), 1) + ")");
      Serial.println("Current drift from average: " + String(drift, 1) + "m");
      
      LoRa.beginPacket();
      LoRa.print(message);
      LoRa.endPacket();
      
      Serial.println(" Update signal sent!");
      Serial.println("=====================================\n");

      // Update averaging with new position (rolling average)
      // This keeps the average updated if the boat is actually moving
      if (drift > 5.0) {  // If drifted more than 5m, start new averaging
        Serial.println("🚢 Significant drift detected - restarting averaging");
        avgLat = 0.0;
        avgLng = 0.0;
        avgCount = 0;
        avgComplete = false;
        firstSignalSent = false;
        confirmationSent = false;
      }

      delay(10000); // Send every 10 seconds while water detected
    }
    
  } else {
    // Show why we're not sending
    if (water == LOW) {
      Serial.println(" No water detected");
    } else if (!gps.location.isValid()) {
      Serial.println(" Water detected but waiting for GPS fix...");
    }
    delay(1000);
  }
}

void displayGPSStatus() {
  Serial.println("\n--- GPS STATUS ---");

  if (gps.location.isValid()) {
    Serial.println(" GPS Fix: VALID");
    Serial.println("  Lat: " + String(gps.location.lat(), 6));
    Serial.println("  Lng: " + String(gps.location.lng(), 6));
    Serial.println("  Satellites: " + String(gps.satellites.value()));

    if (gps.hdop.isValid()) {
      Serial.println("  HDOP: " + String(gps.hdop.hdop(), 1));

      // GPS quality assessment
      bool gpsAccuracyGood = (gps.hdop.hdop() < 5.0 && gps.satellites.value() >= 5) || (gps.satellites.value() >= 7);
      Serial.println("  Quality: " + String(gpsAccuracyGood ? "GOOD ✓" : "POOR ✗"));
    }

    if (gps.altitude.isValid()) {
      Serial.println("  Altitude: " + String(gps.altitude.meters(), 1) + "m");
    }

    if (gps.speed.isValid()) {
      Serial.println("  Speed: " + String(gps.speed.mps(), 1) + " m/s");
    }

    // Show averaging status
    if (avgCount > 0 && !avgComplete) {
      Serial.println("\n GPS Averaging Progress: " + String(avgCount) + "/" + String(AVG_SAMPLES));
      double tempAvgLat = avgLat / avgCount;
      double tempAvgLng = avgLng / avgCount;
      Serial.println("  Current avg: " + String(tempAvgLat, 6) + ", " + String(tempAvgLng, 6));
    } else if (avgComplete) {
      Serial.println("\n GPS Averaging: COMPLETE");
      Serial.println("  Final avg: " + String(avgLat / AVG_SAMPLES, 6) + ", " + String(avgLng / AVG_SAMPLES, 6));
    }
  } else {
    Serial.println(" Waiting for GPS fix...");
    Serial.println("  Satellites in view: " + String(gps.satellites.value()));

    if (gps.charsProcessed() < 10) {
      Serial.println("  ⚠  No GPS data - check wiring!");
    } else {
      Serial.println("  Characters processed: " + String(gps.charsProcessed()));
      Serial.println("  Take the device outside for better signal");
    }
  }

  // Water sensor status
  Serial.println("\n Water Sensor: " + String(digitalRead(WATER_SENSOR_PIN) == HIGH ? "WET" : "DRY"));
  Serial.println("------------------\n");
}