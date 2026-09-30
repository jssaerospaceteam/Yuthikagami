#pragma once

// --- Hardware Pins ---
#define PIN_BMP_SDA     21
#define PIN_BMP_SCL     22

#define PIN_LORA_MISO   19
#define PIN_LORA_MOSI   23
#define PIN_LORA_SCK    18
#define PIN_LORA_NSS    5
#define PIN_LORA_RST    14
#define PIN_LORA_DIO0   26

#define PIN_LED_SENS    27  // Yellow — BMP280 / sensor alive
#define PIN_LED_TX      25  // Red    — LoRa transmit pulse
#define PIN_LED_STATUS  33  // Blue   — system status / heartbeat
#define PIN_BUZZER      4
#define PIN_SERVO       13  // Parachute Ejection Servo Pin

// --- Servo Configuration ---
#define SERVO_LOCKED_ANGLE     0
#define SERVO_EJECT_ANGLE      90
#define SERVO_LOCK_DELAY_MS    10000  

// --- LoRa Settings (Needed for Ground Station) ---
#define LORA_FREQ         434.5E6 
#define LORA_TX_POWER              17
#define LORA_TX_POWER_RECOVERY     10 
#define LORA_BANDWIDTH    125E3
#define LORA_SPREADFACTOR 7
#define LORA_CODINGRATE   5
#define LORA_PREAMBLE     8
#define LORA_SYNC_WORD    0x12

// --- Flight Dynamics & Apogee Thresholds ---
#define LAUNCH_ALT_THRESHOLD   5.0f
#define APOGEE_FALL_THRESHOLD  2.0f
#define TELEMETRY_INTERVAL_MS            500
#define TELEMETRY_INTERVAL_RECOVERY_MS   1000
#define ROCKET_ID              37

// --- Recovery beacon (post-ejection, non-blocking) ---
#define BEEP_BOOT_MS           1000
#define BEEP_RECOVERY_ON_MS    500
#define BEEP_RECOVERY_OFF_MS   500
#define BMP_WARMUP_MS          2000
#define APOGEE_ARM_DELAY_MS    30000

// --- Legacy Variables (To prevent your old code from crashing) ---
#define FILTER_EMA_ALPHA       0.2f
#define APOGEE_PERSISTENCE     3
#define APOGEE_BURST_COUNT     5
#define APOGEE_BURST_INTERVAL  50