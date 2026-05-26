// secrets.example.h - COMMIT THIS TEMPLATE.
// Copy to secrets.h (which is gitignored) and fill in real values locally.
// Never commit real UIDs or the Gate ESP32 MAC if you treat it as private.
#pragma once
#include <stdint.h>

// STA MAC of the Gate ESP32 (TTGO T8). Find via WiFi.macAddress() on that board.
const uint8_t GATE_ESP32_MAC[6] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

// Optional: a UID for solo bring-up before runtime provisioning exists.
// Leave zeroed in the template.
// const uint8_t BRINGUP_UID[6] = { 0,0,0,0,0,0 };
