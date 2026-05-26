// fhss.cpp - STUB. Port the real algorithm from ExpressLRS.
//
// Reference: ExpressLRS/ExpressLRS  src/lib/FHSS/FHSS.cpp + FHSS.h
//   - FHSSrandomSeed is derived from the UID.
//   - The hop sequence is a shuffled list over the region's channel set.
//   - 2.4GHz (SX1280) uses the ISM 2400 band channel table.
//
// Porting checklist:
//   [ ] Copy the rng (a deterministic LCG/xorshift matching ELRS exactly).
//   [ ] Copy the channel base frequency + step for the 2.4GHz table.
//   [ ] Match FHSS_CHANNEL_COUNT to the ELRS value for this region.
//   [ ] Unit-test: a known UID must reproduce the same sequence ELRS prints.

#include "fhss.h"
#include "config.h"

static uint8_t  s_sequence[FHSS_CHANNEL_COUNT];
static uint16_t s_length = 0;

// 2.4GHz ELRS base/step - PLACEHOLDER, verify against ELRS source.
static const uint32_t FHSS_FREQ_BASE = 2400400000UL; // Hz, verify
static const uint32_t FHSS_FREQ_STEP =     1000000UL; // Hz, verify

void fhssGenerate(const uint8_t uid[6]) {
    // TODO: replace with ELRS-accurate seed + shuffle.
    (void)uid;
    for (uint16_t i = 0; i < FHSS_CHANNEL_COUNT; i++) s_sequence[i] = i;
    s_length = FHSS_CHANNEL_COUNT;
}

uint8_t fhssChannelAt(uint16_t hopIndex) {
    if (s_length == 0) return 0;
    return s_sequence[hopIndex % s_length];
}

uint32_t fhssFreqHz(uint8_t channelIndex) {
    return FHSS_FREQ_BASE + (uint32_t)channelIndex * FHSS_FREQ_STEP;
}

uint16_t fhssSequenceLength() { return s_length; }
