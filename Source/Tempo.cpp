/*
  ==============================================================================

    Tempo.cpp
    Created: 18 Nov 2025 3:28:33pm
    Author:  Jesus Herrera

  ==============================================================================
*/

#include "Tempo.h"

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
