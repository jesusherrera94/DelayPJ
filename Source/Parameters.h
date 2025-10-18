/*
  ==============================================================================

    Parameters.h
    Created: 17 Oct 2025 9:49:41pm
    Author:  Jesus Herrera

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

const juce::ParameterID gainParamID { "gain", 1};

class Parameters{
    public:
    Parameters(juce::AudioProcessorValueTreeState& apvts);
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioParameterFloat* gainParam;
};
