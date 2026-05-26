#pragma once
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include "pilots.h"

// Packet sent by each EP1 sniffer to the Gate ESP32 via ESP-NOW.
// Must match GateEP1Packet_t in src/gate_ep1/config.h exactly.
typedef struct __attribute__((packed)) {
    uint8_t  pilot_uid[6];   // ELRS bind UID this measurement belongs to
    int8_t   rssi;           // measured RSSI (dBm)
    uint8_t  lq;             // link quality 0-100 (0 = unused)
    uint32_t ts;             // sniffer millis() timestamp
} GateEP1Packet_t;

extern QueueHandle_t packetQueue;  // queue of GateEP1Packet_t

void setupEspNowGate();
