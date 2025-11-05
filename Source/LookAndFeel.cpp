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

void RotaryKnobLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, [[maybe_unused]] int height, float sliderPos, float rotaryStartAngle, float rotaryEndAngle, juce::Slider &slider) {
    auto bounds = juce::Rectangle<int>(x,y,width,width).toFloat();
    auto knobRect = bounds.reduced(10.0f, 10.0f);
    auto path = juce::Path();
    path.addEllipse(knobRect);
    dropShadow.drawForPath(g, path);
    g.setColour(Colors::knob::outline);
    g.fillEllipse(knobRect);
    
    auto innerRect = knobRect.reduced(2.0f, 2.0f);
    auto gradient = juce::ColourGradient(
                                         Colors::knob::gradientTop, 0.0f, innerRect.getY(), Colors::knob::gradientBottom, 0.0f, innerRect.getBottom(), false
                                         );
    g.setGradientFill(gradient);
    g.fillEllipse(innerRect);
}
