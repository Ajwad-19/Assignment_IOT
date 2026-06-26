# ACE6263 Smart Room Automation System

![Project Status](https://img.shields.io/badge/status-completed-brightgreen)
![SDG 7](https://img.shields.io/badge/SDG-7-006A4E)
![SDG 3](https://img.shields.io/badge/SDG-3-4B9CD3)
![SDG 11](https://img.shields.io/badge/SDG-11-FD9E3A)

---

## 📋 Table of Contents
1. [Project Overview](#-project-overview)
2. [Team Members](#-team-members)
3. [Hardware Components](#-hardware-components)
4. [System Architecture](#-system-architecture)
5. [Features](#-features)
6. [Pin Connections](#-pin-connections)
7. [How to Set Up](#-how-to-set-up)
8. [Repository Structure](#-repository-structure)
9. [GitHub Commit History](#-github-commit-history)
10. [Video Demonstration](#-video-demonstration)
11. [References](#-references)

---

## 🌟 Project Overview

This project implements a **Smart Room Automation System** using an ESP32 microcontroller to address **three United Nations Sustainable Development Goals (SDGs)**:

| SDG | Goal | How Our Project Contributes |
|-----|------|----------------------------|
| **SDG 7** | Affordable and Clean Energy | Reduces electrical waste by automatically turning off lights when not needed |
| **SDG 3** | Good Health and Well-being | Ensures safer indoor air quality through smoke detection and ventilation |
| **SDG 11** | Sustainable Cities and Communities | Paves the way for smart, energy-efficient city infrastructure |

### Problem Statement
Traditional room management systems often operate independently:
- ❌ Lighting stays on regardless of occupancy
- ❌ Ventilation ignores air quality
- ❌ Safety hazards like smoke or extreme heat go unnoticed until too late

### Our Solution
This prototype integrates **4 sensors** with **4 actuators** to create a cohesive, automated system that:
- ✅ Detects human presence and adjusts lighting accordingly
- ✅ Monitors air quality and activates ventilation when needed
- ✅ Alerts occupants to smoke or gas hazards immediately
- ✅ Displays real-time sensor data on an OLED screen

---

## 👥 Team Members

| Name | Student ID | Role | Contributions |
|------|------------|------|---------------|
| Haziq Haikal | [Your ID] | Hardware Assembly & Sensor Integration | Wired all components, calibrated LDR and MQ-2 sensors, implemented relay control functions |
| Ajwad | [Your ID] | Firmware Logic & Timing Management | Wrote main loop logic, implemented PIR and servo control algorithm, non-blocking timing |
| Fayyadh | [Your ID] | System Integration & User Interface | Designed OLED display layout, managed serial output, performed system testing |
| Dannish Luqman | 1231302316 | Documentation & Version Control | Created Bill of Materials, managed GitHub repository, edited video demonstration |

*Total Team Size: 4 members*

---

## 🔧 Hardware Components

| Component | Quantity | Purpose |
|-----------|----------|---------|
| ESP32 Development Board | 1 | Main microcontroller |
| PIR Motion Sensor | 1 | Human presence detection |
| DHT11 Temperature/Humidity Sensor | 1 | Monitor room temperature and humidity |
| LDR Photoresistor | 1 | Detect ambient light levels |
| MQ-2 Gas/Smoke Sensor | 1 | Detect smoke and gas leaks |
| SG90 Servo Motor | 1 | Motion-activated latch mechanism |
| Passive Buzzer | 1 | Audible alarm for hazards |
| 5V Relay Module (4-Channel) | 1 | Control high-power actuators |
| Cooler Fan | 1 | Exhaust ventilation |
| OLED Display (I2C 128x64) | 1 | Real-time user interface |
| Breadboard | 1 | Prototyping platform |
| Jumper Wires | 1 pack | Electrical connections |
| Micro USB Cable | 1 | Power and programming |

---

## 🏗️ System Architecture

### Block Diagram
