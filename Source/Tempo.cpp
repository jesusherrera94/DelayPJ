/*
  ==============================================================================

    Tempo.cpp
    Created: 18 Nov 2025 3:28:33pm
    Author:  Jesus Herrera

  ==============================================================================
*/

#include "Tempo.h"

static std::array<double, 16> noteLenghtMultipliers = {
    0.125,       // 0 = 1/32
    0.5 / 3.0,   // 1 =  1/16 trip
    0.1875,      // 2 = 1/32 dot
    0.25,        // 3 =  1/16
    1.0 / 3.0,   // 4 = 1/8 trip
    0.375,       // 5 = 1/16 dot
    0.5,         // 6 = 1/8
    2.0 / 3.0,   // 7 = 1/4 trip
    0.75,        // 8 = 1/8 dot
    1.0,         // 9 = 1/4
    4.0 / 3.0,   // 10 = 1/2 trip
    1.5,         // 11 = 1/4 dot
    2.0,         // 12 = 1/2
    8.0 / 3.0,   // 13 = 1/1 trip
    3.0 ,        // 14 = 1/2 dot
    4.0          // 15 = 1/1
};

void Tempo::reset() noexcept {
    bpm = 120.0f;
}

void Tempo::update(const juce::AudioPlayHead* playhead) noexcept {
    reset();
    if (playhead == nullptr) { return; }
    const auto opt = playhead->getPosition();
    if (!opt.hasValue()) { return; }
    
    const auto& pos = *opt;
    if (pos.getBpm().hasValue()) {
        bpm = *pos.getBpm();
    }
}

double Tempo::getMillisecondsForNoteLength(int index) const noexcept {
    return 60000.0  * noteLenghtMultipliers[size_t(index)] / bpm;
}
