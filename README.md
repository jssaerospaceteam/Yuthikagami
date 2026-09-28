# Yuthikagami (Yugami Mark-01) 🚀

Official flight computer and telemetry codebase for the **Yugami Mark-01** sounding rocket, developed by **jssaerospaceteam** for the **Akshyaan Spardha 2026** competition.

This repository contains the embedded software designed for automated flight state monitoring, sensor filtering, and reliable parachute recovery subsystems using an **I-Class motor** configuration.

---

## 📂 Project Architecture

The project is structured as a standard **PlatformIO** configuration:

```text
├── include/
│   ├── Config.h             # Pin assignments, constants, and thresholds
│   └── Protocol.h           # Serial and telemetry communication packets
├── src/
│   ├── flight/              # Avionics Firmware
│   │   ├── FlashLogger.h    # Non-volatile high-frequency flight data storage
│   │   ├── SensorFilter.h   # Moving average and noise-suppression algorithms
│   │   └── main.cpp         # Flight state machine and recovery control execution
│   └── ground/              # Telemetry Station Software
│       ├── DashboardHTML.h  # Embedded GUI layout for live flight monitoring
│       └── main.cpp         # Ground station receiver control loop
├── tools/
│   └── ground_logger.py     # Python script to parse serial telemetry and log to CSV
├── platformio.ini           # Build environment, library definitions, and target specs
├── .gitignore               # Excludes build logs and temporary workspace files
├── LICENSE                  # MIT open-source license
└── README.md                # System documentation
```

---

## ⚡ Core Avionics Modules

### 📡 Flight Subsystem (`src/flight/`)
* **State Machine (`main.cpp`):** Manages flight transitions through sequential stages: `STANDBY` ➔ `POWERED_ASCENT` ➔ `COASTING` ➔ `APOGEE_DETECTION` ➔ `DESCENT` ➔ `LANDED`.
* **Noise Mitigation (`SensorFilter.h`):** Uses mathematical filtering to process barometric data, eliminating pressure transients and preventing premature pyrotechnic deployment.
* **Non-Volatile Logging (`FlashLogger.h`):** Ensures crucial black-box flight data (altitude, acceleration, state changes) is securely preserved to onboard flash memory.

### 🎛️ Telemetry & Ground Control (`src/ground/` & `tools/`)
* **Live Monitoring:** The ground unit captures real-time data transmissions over radio links, passing parameters seamlessly into a responsive web dashboard format via `DashboardHTML.h`.
* **Data Persistence:** The companion script `ground_logger.py` actively streams incoming telemetry, auto-formatting records directly into clean data files for post-flight analysis.

---

## 🛡️ Recovery & Ejection Sequence

To guarantee a safe recovery, the deployment logic relies on verified tracking across three strict parameters:
1. **Launch Confirmation:** Triggered by high acceleration (\(a_z \ge \text{Threshold}\)) matching expected I-Class motor ignition dynamics.
2. **Apogee Filtering:** Continually updates peak tracking via a rolling buffer. A descent condition is validated only when a consistent reduction in altitude is maintained across consecutive frames (\(v_z \le 0\)).
3. **Ejection Execution:** Fires the recovery channel to actuate the deployment mechanism cleanly at apex, stabilizing descent speed.

---

## 🛠️ Build and Installation

This repository is built using the **PlatformIO** ecosystem. 

1. Install **Visual Studio Code** and the **PlatformIO IDE** extension.
2. Clone this repository to your local development workspace.
3. Open the root folder in VS Code.
4. Connect your flight controller via USB.
5. Click **Build** and then **Upload** using the PlatformIO toolbar.

---

## 👥 Engineering Team
Maintained with pride by the **jssaerospaceteam**.

## 📜 License
This project is officially open-source and distributed under the terms of the [MIT License](LICENSE).

