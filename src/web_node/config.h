#pragma once
#include <IPAddress.h>

// UART to Gate Node
#define GATE_RX_PIN   3
#define GATE_TX_PIN   2
#define GATE_BAUD     460800

// Pilot limits
#define MAX_REGISTERED  20
#define MAX_ACTIVE       8
#define MAX_LAPS       500
#define DEFAULT_ENTER  (-80)
#define DEFAULT_EXIT   (-90)

// Scan
#define MAX_SCAN_MACS 8

// WiFi Access Point
static const char*     AP_SSID    = "ESP-NOW-LT";
static const char*     AP_PASS    = "esp-now-lt";
static const IPAddress AP_IP      (20, 0, 0, 1);
static const IPAddress AP_GATEWAY (20, 0, 0, 1);
static const IPAddress AP_SUBNET  (255, 255, 255, 0);
