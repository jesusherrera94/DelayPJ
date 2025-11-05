/*
  ==============================================================================

    LookAndFeel.cpp
    Created: 3 Nov 2025 5:37:52pm
    Author:  Jesus Herrera

  ==============================================================================
*/

#include "LookAndFeel.h"

const juce::Typeface::Ptr Fonts::typeface = juce::Typeface::createSystemTypefaceFor(BinaryData::LatoMedium_ttf, BinaryData::LatoMedium_ttfSize);

juce::Font Fonts::getFont(float height) {
    juce::FontOptions options(typeface);
    return options.withMetricsKind(juce::TypefaceMetricsKind::legacy).withHeight(height);
}

RotaryKnobLookAndFeel::RotaryKnobLookAndFeel() {
    setColour(juce::Label::textColourId, Colors::knob::label);
    setColour(juce::Slider::textBoxTextColourId, Colors::knob::label);
    setColour(juce::Slider::rotarySliderFillColourId, Colors::knob::trackActive);
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
    
    auto center = bounds.getCentre();
    auto radius = bounds.getWidth() / 2.0f;
    auto lineWidth = 3.0f;
    auto arcRadius = radius - lineWidth/2.0f;
    juce::Path backgroundArc;
    backgroundArc.addCentredArc(center.x, center.y, arcRadius, arcRadius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
    auto strokeType = juce::PathStrokeType(lineWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded);
    g.setColour(Colors::knob::trackBackground);
    g.strokePath(backgroundArc, strokeType);
    
    auto dialRadius = innerRect.getHeight() / 2.0f - lineWidth;
    auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    juce::Point<float> dialStart(center.x + 10.0f * std::sin(toAngle), center.y - 10.0f * std::cos(toAngle));
    juce::Point<float> dialEnd(center.x + dialRadius * std::sin(toAngle), center.y - dialRadius * std::cos(toAngle));
    juce::Path dialPath;
    dialPath.startNewSubPath(dialStart);
    dialPath.lineTo(dialEnd);
    g.setColour(Colors::knob::dial);
    g.strokePath(dialPath, strokeType);
    
    if (slider.isEnabled()) {
        float fromAngle = rotaryStartAngle;
        if (slider.getProperties()["drawFromMiddle"]){
            fromAngle += (rotaryEndAngle - rotaryStartAngle) / 2.0f;
        }
        juce::Path valueArc;
        valueArc.addCentredArc(center.x, center.y, arcRadius, arcRadius, 0.0f, fromAngle, toAngle, true);
        g.setColour(slider.findColour(juce::Slider::rotarySliderFillColourId));
        g.strokePath(valueArc, strokeType);
    }
}

juce::Font RotaryKnobLookAndFeel::getLabelFont([[maybe_unused]]juce::Label& label) {
    return Fonts::getFont();
}
