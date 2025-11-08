<h1 align="center">🔐 ESP32 Security System</h1>

<p align="center">
  <b>Smart IoT Surveillance using ESP32-CAM & ESP32 DevKit</b><br>
  <sub>📸 Real-time Monitoring | 💡 Laser Tripwire | 👀 PIR Motion | ✉️ Email Alerts | 🌐 Web Dashboard</sub>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/Platform-ESP32--CAM-blue?style=flat-square">
  <img src="https://img.shields.io/badge/Framework-Arduino-green?style=flat-square">
  <img src="https://img.shields.io/badge/SMTP-Brevo-orange?style=flat-square">
  <img src="https://img.shields.io/badge/License-MIT-lightgrey?style=flat-square">
</p>

---

## 🚀 Overview

This project combines **ESP32-CAM** and **ESP32 DevKit** to build a complete IoT-based security system with:
- Live image capture
- Real-time motion & laser detection
- Web-based security dashboard
- Automated email alerts with photos via **Brevo SMTP**

> 💡 Ideal for smart home, office, and lab security setups.

---

## 🧩 Features

| Feature | Description |
|----------|-------------|
| 👁️ **PIR Motion Detection** | Detects human movement |
| 🔦 **Laser Tripwire** | Triggers when the laser beam is cut |
| 📸 **ESP32-CAM Integration** | Captures and stores photos instantly |
| 🌐 **Live Web Dashboard** | Monitor logs & latest captured image |
| 🕒 **NTP Time Sync (IST)** | Real-time timestamps on all events |
| ✉️ **Email Notifications** | Sends photo via Brevo SMTP |
| 🔔 **Buzzer Alert** | Audible intruder alarm |

---

## 🧠 System Architecture

```text
┌──────────────────────────┐
│        ESP32 DevKit       │
│ ───────────────────────── │
│ PIR Sensor → Motion Detect│
│ LDRs (x3) → Laser Detect  │
│ Buzzer → Alert            │
│ WiFi → Send HTTP event    │
└────────────┬─────────────┘
             │
             ▼
┌──────────────────────────┐
│        ESP32-CAM         │
│ ───────────────────────── │
│ Receives event trigger   │
│ Captures photo (JPEG)    │
│ Sends email via Brevo    │
│ Hosts live dashboard     │
└──────────────────────────┘
