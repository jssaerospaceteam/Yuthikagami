#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <LoRa.h>
#include <Adafruit_BMP280.h>
#include <ESP32Servo.h>
#include <ESP32PWM.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "rom/rtc.h"

#include "Config.h"
#include "Protocol.h"

#define APOGEE_LATCH_MAGIC 0xA90EEE01u

Adafruit_BMP280 bmp;
Servo parachuteServo;

RTC_DATA_ATTR uint32_t apogeeLatch = 0;

float groundAltitude = 0.0f;
float currentAltitude = 0.0f;
float maxAltitude = 0.0f;
bool parachuteDeployed = false;
FlightState currentState = STATE_BOOT;

uint16_t packetSequence = 0;
unsigned long lastTxTime = 0;
unsigned long lastServoHold = 0;

bool recoveryBeaconActive = false;
bool beeperOn = false;
unsigned long lastBeepToggle = 0;

bool statusLedOn = false;
unsigned long lastStatusBlink = 0;

static void setBeeper(bool on) {
    beeperOn = on;
    digitalWrite(PIN_BUZZER, on ? HIGH : LOW);
}

static void setStatusLed(bool on) {
    statusLedOn = on;
    digitalWrite(PIN_LED_STATUS, on ? HIGH : LOW);
}

static void blinkForever(uint8_t pin) {
    while (1) {
        digitalWrite(pin, HIGH);
        delay(150);
        digitalWrite(pin, LOW);
        delay(150);
    }
}

static void waitToLockServo() {
    Serial.println("Servo unlocked — fit it in the tube. Locking at 0 deg in 10 s...");
    const unsigned long start = millis();
    while (millis() - start < SERVO_LOCK_DELAY_MS) {
        setStatusLed(((millis() - start) / 250) % 2 == 0);
        delay(50);
    }
    parachuteServo.attach(PIN_SERVO, 500, 2400);
    parachuteServo.write(SERVO_LOCKED_ANGLE);
    setStatusLed(true);
    Serial.println("Servo locked at 0 deg");
}

static void enterDescentMode(bool fromReset) {
    parachuteDeployed = true;
    apogeeLatch = APOGEE_LATCH_MAGIC;
    currentState = STATE_DESCENT;
    recoveryBeaconActive = true;
    parachuteServo.write(SERVO_EJECT_ANGLE);
    LoRa.setTxPower(LORA_TX_POWER_RECOVERY);
    lastBeepToggle = millis();
    lastServoHold = millis();
    setBeeper(true);
    if (!fromReset) {
        Serial.println("[WARNING] APOGEE DETECTED - PARACHUTE EJECTED!");
    } else {
        Serial.println("[INFO] Restored descent mode after reset");
    }
}

void triggerParachute() {
    if (parachuteDeployed) {
        return;
    }
    enterDescentMode(false);
}

void updateRecoveryBeacon() {
    if (!recoveryBeaconActive) {
        return;
    }

    const unsigned long interval = beeperOn ? BEEP_RECOVERY_ON_MS : BEEP_RECOVERY_OFF_MS;
    if (millis() - lastBeepToggle >= interval) {
        lastBeepToggle = millis();
        setBeeper(!beeperOn);
    }
}

void holdEjectServo() {
    if (!parachuteDeployed) {
        return;
    }
    if (millis() - lastServoHold >= 200) {
        lastServoHold = millis();
        parachuteServo.write(SERVO_EJECT_ANGLE);
    }
}

void transmitTelemetry(float altMeters) {
    uint8_t packet[6];
    packet[0] = ROCKET_ID;
    packet[1] = parachuteDeployed ? PKT_TYPE_APOGEE : PKT_TYPE_NORMAL_ALTITUDE;
    packet[2] = (uint8_t)((packetSequence >> 8) & 0xFF);
    packet[3] = (uint8_t)(packetSequence & 0xFF);

    float clampedAlt = altMeters;
    if (clampedAlt < 0.0f) {
        clampedAlt = 0.0f;
    }

    uint32_t altitudeDm = (uint32_t)(clampedAlt * 10.0f + 0.5f);
    if (altitudeDm > 0xFFFF) {
        altitudeDm = 0xFFFF;
    }

    packet[4] = (uint8_t)((altitudeDm >> 8) & 0xFF);
    packet[5] = (uint8_t)(altitudeDm & 0xFF);

    digitalWrite(PIN_LED_TX, HIGH);
    LoRa.beginPacket();
    LoRa.write(packet, sizeof(packet));
    LoRa.endPacket();
    digitalWrite(PIN_LED_TX, LOW);

    packetSequence++;
}

void setup() {
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
    setCpuFrequencyMhz(160);

    const RESET_REASON resetReason = rtc_get_reset_reason(0);
    const bool powerOn = (resetReason == POWERON_RESET);
    if (powerOn) {
        apogeeLatch = 0;
    }
    const bool restoreDescent = (!powerOn && apogeeLatch == APOGEE_LATCH_MAGIC);

    Serial.begin(115200);
    Serial.print("Reset reason CPU0: ");
    Serial.println((int)resetReason);

    pinMode(PIN_BUZZER, OUTPUT);
    pinMode(PIN_LED_STATUS, OUTPUT);
    pinMode(PIN_LED_SENS, OUTPUT);
    pinMode(PIN_LED_TX, OUTPUT);
    setBeeper(false);
    digitalWrite(PIN_LED_STATUS, LOW);
    digitalWrite(PIN_LED_SENS, LOW);
    digitalWrite(PIN_LED_TX, LOW);

    ESP32PWM::allocateTimer(0);
    parachuteServo.setPeriodHertz(50);
    if (restoreDescent) {
        parachuteServo.attach(PIN_SERVO, 500, 2400);
        parachuteServo.write(SERVO_EJECT_ANGLE);
    } else {
        waitToLockServo();
    }

    Wire.begin(PIN_BMP_SDA, PIN_BMP_SCL);
    if (!bmp.begin(0x76) && !bmp.begin(0x77)) {
        Serial.println("BMP280 initialization failed!");
        blinkForever(PIN_LED_SENS);
    }
    digitalWrite(PIN_LED_SENS, HIGH);

    bmp.setSampling(
        Adafruit_BMP280::MODE_NORMAL,
        Adafruit_BMP280::SAMPLING_X2,
        Adafruit_BMP280::SAMPLING_X16,
        Adafruit_BMP280::FILTER_X16,
        Adafruit_BMP280::STANDBY_MS_63
    );

    delay(BMP_WARMUP_MS);

    float sum = 0.0f;
    for (int i = 0; i < 10; i++) {
        sum += bmp.readAltitude(1013.25);
        delay(100);
    }
    groundAltitude = sum / 10.0f;
    currentState = STATE_READY;

    SPI.begin(PIN_LORA_SCK, PIN_LORA_MISO, PIN_LORA_MOSI, PIN_LORA_NSS);
    LoRa.setPins(PIN_LORA_NSS, PIN_LORA_RST, PIN_LORA_DIO0);
    if (!LoRa.begin(LORA_FREQ)) {
        Serial.println("LoRa init failed!");
        blinkForever(PIN_LED_TX);
    }

    LoRa.setTxPower(restoreDescent ? LORA_TX_POWER_RECOVERY : LORA_TX_POWER);
    LoRa.setSignalBandwidth(LORA_BANDWIDTH);
    LoRa.setSpreadingFactor(LORA_SPREADFACTOR);
    LoRa.setCodingRate4(LORA_CODINGRATE);
    LoRa.setPreambleLength(LORA_PREAMBLE);
    LoRa.setSyncWord(LORA_SYNC_WORD);
    LoRa.enableCrc();

    lastTxTime = millis();
    lastStatusBlink = millis();
    setStatusLed(true);

    if (restoreDescent) {
        enterDescentMode(true);
        Serial.println("Yuthikagami Mk-1 recovery restored");
    } else {
        setBeeper(true);
        delay(BEEP_BOOT_MS);
        setBeeper(false);
        Serial.println("Yuthikagami Mk-1 Flight System Ready!");
    }
}

void loop() {
    currentAltitude = bmp.readAltitude(1013.25) - groundAltitude;

    if (currentAltitude > maxAltitude) {
        maxAltitude = currentAltitude;
    }

    if (!parachuteDeployed &&
        millis() > APOGEE_ARM_DELAY_MS &&
        maxAltitude > LAUNCH_ALT_THRESHOLD) {
        currentState = STATE_ASCENT;
        if ((maxAltitude - currentAltitude) >= APOGEE_FALL_THRESHOLD) {
            triggerParachute();
        }
    }

    holdEjectServo();
    updateRecoveryBeacon();

    const unsigned long statusInterval = parachuteDeployed ? 200UL : 1000UL;
    if (millis() - lastStatusBlink >= statusInterval) {
        lastStatusBlink = millis();
        setStatusLed(!statusLedOn);
    }

    const unsigned long txInterval = parachuteDeployed ? TELEMETRY_INTERVAL_RECOVERY_MS : TELEMETRY_INTERVAL_MS;
    if (millis() - lastTxTime >= txInterval) {
        if (parachuteDeployed && beeperOn) {
            return;
        }
        lastTxTime = millis();
        transmitTelemetry(currentAltitude);
    }
}
