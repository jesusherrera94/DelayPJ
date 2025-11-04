/*
  ==============================================================================

    LookAndFeel.cpp
    Created: 3 Nov 2025 5:37:52pm
    Author:  Jesus Herrera

  ==============================================================================
*/

#include "LookAndFeel.h"

RotaryKnobLookAndFeel::RotaryKnobLookAndFeel() {
    setColour(juce::Label::textColourId, Colors::knob::label);
    setColour(juce::Slider::textBoxTextColourId, Colors::knob::label);
}

void RotaryKnobLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos, float rotaryStartAngle, float rotaryEndAngle, juce::Slider &slider) {
    
}
