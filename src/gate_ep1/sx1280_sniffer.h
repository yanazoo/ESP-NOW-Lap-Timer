// sx1280_sniffer.h - minimal SX1280 control for passive RSSI sniffing
#pragma once
#include <stdint.h>

// Bring up SPI to the EP1's onboard SX1280 and put it in a known LoRa config
// matching ELRS (bandwidth, spreading factor, coding rate for the packet rate
// in use). Returns true on successful chip ID read.
bool sxBegin();

// Park the radio on a specific RF frequency (Hz) and enter continuous RX.
void sxSetFrequencyHz(uint32_t freqHz);

// Read instantaneous RSSI (dBm, negative). Valid after a packet or in RX.
int8_t sxReadRssi();

// True if a packet was received since the last check (sync detection).
bool sxPacketReceived();
