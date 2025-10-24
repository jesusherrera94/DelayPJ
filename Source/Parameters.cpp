/*
  ==============================================================================

    Parameters.cpp
    Created: 17 Oct 2025 9:49:41pm
    Author:  Jesus Herrera

  ==============================================================================
*/

#include "Parameters.h"

template<typename T>
static void castParameter(juce::AudioProcessorValueTreeState& apvts, const juce::ParameterID& id, T& destination) {
    destination = dynamic_cast<T>(apvts.getParameter(id.getParamID()));
    jassert(destination);
}

Parameters::Parameters(juce::AudioProcessorValueTreeState& apvts){
    castParameter(apvts, gainParamID, gainParam);
    castParameter(apvts, delayTimeParamID, delayTimeParam);
}

juce::AudioProcessorValueTreeState::ParameterLayout Parameters::createParameterLayout() {
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add(
               std::make_unique<juce::AudioParameterFloat>(
                                                           gainParamID,
                                                           "Output Gain",
                                                           juce::NormalisableRange<float>{ -12.0f, 12.0f},
                                                           0.0f
                                                           )
               );
    layout.add(
               std::make_unique<juce::AudioParameterFloat>(
                                                           delayTimeParamID,
                                                           "Delay Time",
                                                           juce::NormalisableRange<float> {minDelayTime, maxDelayTime, 0.001f, 0.25f },
                                                           100.0f
                                                           )
               );
    return layout;
}

void Parameters::update() noexcept {
    gainSmoother.setTargetValue(juce::Decibels::decibelsToGain(gainParam->get()));
    delayTime = delayTimeParam->get();
}

void Parameters::prepareToPlay(double sampleRate) noexcept {
    double duration = 0.2f;
    gainSmoother.reset(sampleRate, duration);
}

void Parameters::reset() noexcept {
    gain = 0.0f;
    gainSmoother.setCurrentAndTargetValue(juce::Decibels::decibelsToGain(gainParam->get()));
    delayTime = 0.0f;
}

void Parameters::smoothen() noexcept {
    gain = gainSmoother.getNextValue();
}
