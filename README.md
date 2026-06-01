# 👶 Smart Infant Incubator using ESP32

A smart incubator prototype built using **ESP32**, designed to monitor and regulate temperature, humidity, and air quality in real-time. The system combines sensor fusion, PID-based control, relay automation, and cloud monitoring through Blynk IoT.

---

## 🚀 Highlights

* 🌡️ Dual Temperature Monitoring (DHT22 + DS18B20)
* 🧠 PID-Based Temperature Control
* 💧 Automatic Humidity Regulation
* ☣️ Gas Leakage Detection
* 🔔 Safety Alarm System
* ⚡ 4-Relay Automation
* 📟 20x4 LCD Live Monitoring
* ☁️ Blynk IoT Cloud Dashboard
* 📱 Remote Monitoring from Mobile

---

## 🛠️ Hardware Used

* ESP32 DevKit V1
* DHT22 Sensor
* DS18B20 Sensor
* MQ Gas Sensor
* 20x4 I2C LCD
* 4-Channel Relay Module
* Active Buzzer

---

## ⚙️ System Functions

| Module     | Function                     |
| ---------- | ---------------------------- |
| Heater     | Temperature Control          |
| Fan        | Cooling & Safety Ventilation |
| Humidifier | Humidity Regulation          |
| Pump       | Moisture Support             |
| MQ Sensor  | Gas Detection                |
| Buzzer     | Alarm Notification           |

---

## 📊 Monitoring Parameters

* Temperature
* Humidity
* Gas Level
* PID Output
* Heater Status
* Fan Status
* Alarm Status

---

## ☁️ Live Simulation

🔗 Wokwi Simulation

https://wokwi.com/projects/465599207648705537

---

## 📱 Blynk Dashboard

🔗 Cloud Dashboard

https://blynk.cloud/dashboard/706500/global/devices/215451/organization/706500/devices/2155847/dashboard

---

## 🧠 Control Strategy

* Heater automatically maintains the desired temperature.
* Fan activates during over-temperature, gas alerts, or sensor faults.
* Humidifier and pump regulate humidity within predefined limits.
* Alarm is triggered during unsafe operating conditions.

---

## 🔮 Future Scope

* Push Notifications
* Data Logging
* Historical Trends
* OTA Updates
* Auto/Manual Mode Selection
* Sensor Redundancy

---

## 👨‍💻 Developed By

**Ansh Dubey**
B.Tech – Electronics & Instrumentation Engineering

ESP32 • Embedded Systems • IoT • Automation • Control Systems
