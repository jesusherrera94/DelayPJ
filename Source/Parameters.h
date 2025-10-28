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
const juce::ParameterID delayTimeParamID {"delayTime", 1};
const juce::ParameterID mixParamID {"mix", 1};

class Parameters{
    public:
        Parameters(juce::AudioProcessorValueTreeState& apvts);
        static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
        void update() noexcept;
        void prepareToPlay(double sampleRate) noexcept;
        void reset() noexcept;
        void smoothen() noexcept;
        float gain = 0.0f;
        static constexpr float minDelayTime = 5.0f;
        static constexpr float maxDelayTime = 5000.0f;
        float delayTime;
        float mix = 1.0f;
    private:
        juce::AudioParameterFloat* gainParam;
        juce::LinearSmoothedValue<float> gainSmoother;
        juce::AudioParameterFloat* delayTimeParam;
        float targetDelayTime = 0.0f;
        float coeff = 0.0f; // one-pole smoothing(filter)
        juce::AudioParameterFloat* mixParam;
        juce::LinearSmoothedValue<float> mixSmoother;
    
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Parameters)
};
