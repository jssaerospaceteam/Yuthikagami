#pragma once
#include <stdint.h>

#define PKT_TYPE_NORMAL_ALTITUDE 0x01
#define PKT_TYPE_APOGEE          0x02

#pragma pack(push, 1)
struct TelemetryPacket {
    uint8_t  rocket_id;
    uint8_t  type;
    uint16_t sequence;   
    uint16_t altitude;   
};
#pragma pack(pop)

// Flight States
enum FlightState {
    STATE_BOOT = 0,
    STATE_READY,
    STATE_ASCENT,
    STATE_NEAR_APOGEE,
    STATE_APOGEE_LOCKED,
    STATE_DESCENT,
    STATE_LANDED
};