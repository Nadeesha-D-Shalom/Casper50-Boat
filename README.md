# 🌊 CASPER 50 — IoT-Based Autonomous Rescue Boat

![CASPER 50 Banner](./assets/casper50-banner.png)

**CASPER 50** is an IoT-driven autonomous rescue boat designed to support fishermen during maritime emergencies.
It integrates **ESP32 + LoRa + GPS + Sensors** to ensure **long-range communication, real-time tracking, and automated rescue operations**, even in poor connectivity zones.

---

## 🚀 Project Overview

The CASPER 50 project aims to:

* Provide an **autonomous rescue mechanism** for fishermen in distress.
* Enable **real-time location tracking** via GPS.
* Ensure **long-range communication** using LoRa SX1278 modules.
* Improve safety with **water-level, obstacle detection, and SOS systems**.
* Explore **AI-powered decision-making** for future autonomous navigation.

---

## ✨ Key Features

✅ **Real-time GPS tracking** of boats
✅ **Long-range wireless communication** with LoRa
✅ **Water level & environmental monitoring** (ultrasonic, temperature, soil/water sensors)
✅ **Emergency alert system** — SOS button triggers rescue alerts
✅ **Autonomous control** — DC motors, servos, and solar-powered modules
✅ **Mobile app dashboard** for live monitoring (Android/Java, Retrofit)
✅ **Backend with REST API** + MySQL database
✅ **Research-ready system** — IEEE format paper in progress

---

## 🛠️ Tech Stack

### Hardware

* ESP32 DevKit V1 / WROOM-32U
* LoRa SX1278 Module (433 MHz / 868 MHz)
* GPS Module (NEO-6M / NEO-8M)
* Ultrasonic Sensors (HC-SR04)
* Water Level Sensors (float + ultrasonic)
* DC Gear Motors + L298N Motor Driver
* Servo Motor (umbrella system for solar/wind protection)
* Solar Panels + TP4056 Charging Module + Li-Ion Battery

### Software

* **Arduino IDE (C/C++)** — firmware development
* **Spring Boot (Java)** — backend API
* **MySQL** — database for sensor + boat logs
* **Android App (Java + Jetpack Compose)** — mobile dashboard
* **LoRaWAN Protocol** — long-range communication tuning
* **GitHub Actions** — CI/CD workflows

---

## 📐 System Architecture

![System Diagram](./assets/casper50-architecture.png)

[Full System Design (Google Drive)](https://drive.google.com/) *(replace with actual link)*

---

## ⚙️ Installation & Setup

### 1. Firmware (ESP32 + LoRa)

```bash
# Clone repo
git clone https://github.com/Nadeesha-D-Shalom/Casper50-Boat.git
cd Casper50-Boat/firmware

# Open in Arduino IDE
# Install required libraries:
#   - LoRa by Sandeep Mistry
#   - TinyGPS++
#   - WiFiClientSecure
```

### 2. Backend (Spring Boot)

```bash
cd Casper50-Boat/backend
mvn spring-boot:run
# Runs on: http://localhost:8080/
```

### 3. Mobile App

* Open `Casper50-MobileApp` in Android Studio
* Update backend URL in `ApiClient.java`
* Run on emulator/physical device

---

## 📊 Usage

1. Power on **ESP32 rescue boat unit** → starts GPS + sensor monitoring.
2. **LoRa transmitter** sends data packets to base station.
3. **Backend server** stores data in MySQL.
4. **Android app** fetches & displays:

   * Boat location (Google Maps)
   * Water level, temperature, fan/water pump status
   * SOS alerts in real-time

---

## 📄 Research & Publications

* [IEEE OCEANS Paper Draft](https://ieee.org/) *(in progress)*
* [Sri Lanka Patent Application (Form P01)](https://www.nipo.gov.lk/) *(drafting)*
* [CodeFeast IoT Challenge 2025 Submission](https://codefeast.lk/) *(planned)*

---

## 👥 Contributors

* **[Nadeesha D. Shalom](https://github.com/Nadeesha-D-Shalom)** — Project Lead & Backend Developer
* Team Members: Firmware, Hardware, UI/UX, Database

---

## 🤝 Contributing

We welcome contributions to improve CASPER 50!

1. Fork the repo
2. Create a branch: `feature/your-feature`
3. Commit your changes
4. Open a Pull Request

---

## 📌 Useful Links

* 🔗 [Project Repository](https://github.com/Nadeesha-D-Shalom/Casper50-Boat)
* 🔗 [Backend Repo](https://github.com/Nadeesha-D-Shalom/Casper50-Backend)
* 🔗 [Mobile App Repo](https://github.com/Nadeesha-D-Shalom/Casper50-MobileApp)
* 🔗 [Personal Portfolio](https://github.com/Nadeesha-D-Shalom)
* 🔗 [LinkedIn Profile](https://www.linkedin.com/in/nadeesha-shalom-a5a2a4251/)

---

## 📜 License

This project is licensed under the **MIT License**.
See [LICENSE](./LICENSE) for details.

---

⚡ *CASPER 50 is not just a boat — it’s a step towards safer oceans and smarter IoT rescue systems.* 🌊

