# Yuthikagami (Yugami MK1) 🚀

Official repository for the **Yuthikagami / Yugami MK1** amateur rocketry project participating in **Akshyaan Spardha 2026**. This repository contains the flight computer software, sensory calibration, and automated safety systems.

## 🛠️ Project Overview
This codebase handles the core flight logic for the Yugami MK1 rocket, integrating thrust curve management for the **I Class motor** and a reliable automated system for atmospheric recovery.

### Key Modules:
* **Elementary Control Logic:** Baseline stabilization, flight phase monitoring, and sensor polling.
* **I Class Motor Integration:** Thrust-phase analytics and burnout detection algorithms.
* **Parachute Ejection Sequence:** Automated, altimeter-driven apogee detection triggering dual/single deployment deployment systems.

---

## 📂 Repository Structure
```text
├── src/
│   ├── main.ino            # Main flight computer execution loop
│   ├── sensors.cpp         # Altimeter, IMU, and Barometer calibrations
│   └── recovery.cpp        # Parachute ejection and pyrotechnic logic
├── hardware/               # Wiring schematics and pin configurations
└── README.md               # Project documentation
```

---

## 🚀 Recovery & Ejection Sequence Logic
The system constantly polls the onboard barometric altimeter and accelerometer data to safely manage rocket stages:
1. **Launch Detection:** Triggers upon sudden acceleration spike (> 2G).
2. **Burnout Phase:** Constantly tracks velocity decrease following I-Class motor burnout.
3. **Apogee Detection:** Calculated via a moving average filter checking for peak altitude ($v_z \le 0$).
4. **Ejection Deploy:** Sends a high signal to the pyrotechnic charge/servo channel to release the parachute safely.

---

## 👥 Team
Developed and maintained by **jssaerospaceteam**.

---

## 📜 License
This project is licensed under the [MIT License](LICENSE).
