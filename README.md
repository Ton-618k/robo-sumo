# robo-sumo
# Autonomous Mini Robo-Sumo

An autonomous sumo robot designed for real-time obstacle detection, enemy tracking in dynamic competition environments.

This repository showcases the **latest operational build** of the robot. An improved next-generation version with an upgraded chassis structure and optimized sensor control loops is currently under active development.

---

## 📌 Project Overview & Highlights

- **Custom 3D Chassis:** Designed in FreeCAD and 3D printed for high durability and a low center of gravity.
- **Embedded Control System:** Low-level firmware programmed in C/C++ to process real-time sensor inputs and drive motors via PWM.
- **Sensor Integration:** Uses ultrasonic rangefinders for obstacle detection.
- **Active Iteration:** Building on field testing data from this current design to engineer a faster, more resilient next version.

---

## 🛠️ Hardware & Tech Stack

- **Microcontroller:** Arduino mega 2560 pro
- **Sensors:** Ultrasonic Sensors
- **Actuators & Power:** High-RPM DC Motors, H-Bridge Motor Drivers, LiPo Power System
- **CAD & Prototyping:** FreeCAD (3D Modeling), FDM 3D Printing
- **Software:** C / C++

---

## 📂 Repository Structure

```text
├── firmware/     # C/C++ source code for sensor polling and motor drive logic
├── cad/          # FreeCAD source files (.FCStd) and 3D printable files (.STL)
└── assets/       # Photos of the physical robot and CAD screenshots
