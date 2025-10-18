/*
  ==============================================================================

    Parameters.cpp
    Created: 17 Oct 2025 9:49:41pm
    Author:  Jesus Herrera

  ==============================================================================
*/

#include "Parameters.h"

Parameters::Parameters(juce::AudioProcessorValueTreeState& apvts){
    auto* param = apvts.getParameter(gainParamID.getParamID());
    gainParam = dynamic_cast<juce::AudioParameterFloat*>(param);
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
    return layout;
}
