// config.h - Gate EP1 sniffer (ESP8285 + SX1280)
// Compile-time constants and shared packet definitions.
#pragma once
#include <stdint.h>

// ---- SX1280 SPI pins (ESP8285) ----
// Confirmed from ExpressLRS generic 2.4GHz ESP8285 RX layout.
// HappyModel EP1/EP2 TCXO use this same reference pinout (identical PCB,
// antenna differs only). Verify against your unit before relying on it.
#define SX_PIN_NSS    15   // chip select
#define SX_PIN_SCK    14   // HSPI clock
#define SX_PIN_MOSI   13   // HSPI MOSI
#define SX_PIN_MISO   12   // HSPI MISO
#define SX_PIN_BUSY    5   // SX1280 BUSY
#define SX_PIN_DIO1    4   // SX1280 DIO1 (IRQ)
#define SX_PIN_RST     2   // SX1280 reset
#define PIN_SERIAL_RX  3   // also the bootloader-entry pad (hold LOW at reset)
#define PIN_SERIAL_TX  1
#define PIN_LED       16

// ---- Flashing note ----
// No separate GPIO0 test point is needed. Holding the RX pad (GPIO3) LOW at
// power-on drops the ESP8285 into the UART bootloader. Flash custom firmware
// over the exposed 5V / GND / RX / TX pads with esptool (UART method).
// A solid LED with the TX off = the unit is sitting in bootloader mode.

// ---- ELRS / FHSS ----
#define FHSS_CHANNEL_COUNT   80     // 2.4GHz ELRS; confirm against ELRS source
#define ELRS_SLOT_US         4000   // 250Hz default; adjust per packet rate
#define SX_SWITCH_US         1000   // approx SX1280 frequency switch time

// ---- Lock-on tuning ----
#define SCAN_DWELL_US        1500   // RX dwell per channel during SCAN phase
#define MISS_STREAK_RESYNC   30     // consecutive misses -> drop back to SCAN

// ---- RSSI reporting ----
#define RSSI_REPORT_MS       50     // 20 Hz, matches existing RSSI_INTERVAL_MS

// ---- Identity ----
// 6-byte ELRS bind UID this sniffer follows. Provisioned at runtime
// (UART or ESP-NOW from web node), NOT hardcoded. See secrets.example.h.
typedef struct { uint8_t uid[6]; bool valid; } SnifferIdentity_t;

// ---- ESP-NOW packet: Gate EP1 -> Gate ESP32 ----
// Keep in sync with the matching struct in src/gate_node/promiscuous.*
typedef struct __attribute__((packed)) {
    uint8_t  pilot_uid[6];   // which pilot's EP1 this RSSI belongs to
    int8_t   rssi;           // measured RSSI (dBm)
    uint8_t  lq;             // link quality 0-100 (optional, 0 if unused)
    uint32_t ts;             // sniffer millis() timestamp
} GateEP1Packet_t;

// Gate ESP32 ESP-NOW peer MAC. Set to your TTGO T8 STA MAC.
// Keep the real value in secrets.h (gitignored).
extern const uint8_t GATE_ESP32_MAC[6];
