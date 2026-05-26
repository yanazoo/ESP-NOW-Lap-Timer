// main.cpp - Gate EP1 sniffer entry point.
// State machine: PROVISION -> SCAN -> FOLLOW, reporting RSSI over ESP-NOW.
//
// This is a skeleton wiring the modules together. The TODOs in fhss.cpp and
// sx1280_sniffer.cpp must be completed for real behavior.

#include <Arduino.h>
#include "config.h"
#include "fhss.h"
#include "sx1280_sniffer.h"
#include "espnow_tx.h"

enum State { ST_PROVISION, ST_SCAN, ST_FOLLOW };
static State state = ST_PROVISION;

static SnifferIdentity_t ident = { {0}, false };
static uint16_t hopIndex = 0;
static uint16_t missStreak = 0;
static uint32_t lastReport = 0;

// TODO: receive UID provisioning (UART line or ESP-NOW from web node).
// For bring-up you may temporarily load a UID from secrets.h. Do NOT commit it.
static bool tryProvision() {
    // if (got UID) { memcpy(ident.uid, ..., 6); ident.valid = true; return true; }
    return false;
}

void setup() {
    Serial.begin(115200);
    delay(50);
    Serial.println(F("[gate_ep1] boot"));

    if (!sxBegin())     Serial.println(F("[gate_ep1] SX1280 init FAILED (stub)"));
    if (!espnowBegin()) Serial.println(F("[gate_ep1] ESP-NOW init FAILED"));
}

void loop() {
    switch (state) {
    case ST_PROVISION:
        if (tryProvision()) {
            fhssGenerate(ident.uid);
            hopIndex = 0;
            state = ST_SCAN;
            Serial.println(F("[gate_ep1] provisioned -> SCAN"));
        }
        break;

    case ST_SCAN: {
        // Step through the sequence quickly looking for any packet.
        sxSetFrequencyHz(fhssFreqHz(fhssChannelAt(hopIndex)));
        delayMicroseconds(SCAN_DWELL_US);
        if (sxPacketReceived()) {
            missStreak = 0;
            state = ST_FOLLOW;
            Serial.println(F("[gate_ep1] locked -> FOLLOW"));
        } else {
            hopIndex++;
        }
        break;
    }

    case ST_FOLLOW: {
        sxSetFrequencyHz(fhssFreqHz(fhssChannelAt(hopIndex)));
        delayMicroseconds(ELRS_SLOT_US - SX_SWITCH_US);

        if (sxPacketReceived()) {
            missStreak = 0;
            int8_t rssi = sxReadRssi();
            uint32_t now = millis();
            if (now - lastReport >= RSSI_REPORT_MS) {
                espnowSendRssi(ident.uid, rssi, /*lq*/0, now);
                lastReport = now;
            }
        } else if (++missStreak >= MISS_STREAK_RESYNC) {
            state = ST_SCAN;
            Serial.println(F("[gate_ep1] miss streak -> SCAN"));
        }
        hopIndex++;
        break;
    }
    }
}
