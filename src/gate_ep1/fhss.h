// fhss.h - ELRS FHSS sequence generation + hop tracking
#pragma once
#include <stdint.h>

// Build the deterministic hop sequence from a 6-byte bind UID.
// Port the algorithm from ExpressLRS (src/lib/FHSS). Keep it byte-for-byte
// compatible or the gate will hop to the wrong channels.
void fhssGenerate(const uint8_t uid[6]);

// Channel index (0..FHSS_CHANNEL_COUNT-1) for a given hop position.
uint8_t fhssChannelAt(uint16_t hopIndex);

// Convert a channel index to the SX1280 RF frequency (Hz).
uint32_t fhssFreqHz(uint8_t channelIndex);

// Total number of entries in the generated sequence.
uint16_t fhssSequenceLength();
