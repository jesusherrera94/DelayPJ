/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DelayPJAudioProcessor::DelayPJAudioProcessor() :
    juce::AudioProcessor(
                         BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)
                         )
{
    auto* param = apvts.getParameter(gainParamID.getParamID());
    gainParam = dynamic_cast<juce::AudioParameterFloat*>(param);
}

DelayPJAudioProcessor::~DelayPJAudioProcessor()
{
}

//==============================================================================
const juce::String DelayPJAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool DelayPJAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool DelayPJAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool DelayPJAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double DelayPJAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int DelayPJAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int DelayPJAudioProcessor::getCurrentProgram()
{
    return 0;
}

void DelayPJAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String DelayPJAudioProcessor::getProgramName (int index)
{
    return {};
}

void DelayPJAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void DelayPJAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void DelayPJAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool DelayPJAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainInputChannelSet() == juce::AudioChannelSet::stereo();
}

void DelayPJAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, [[maybe_unused]] juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    float gainInDecibels = gainParam->get();
    float gain = juce::Decibels::decibelsToGain(gainInDecibels);
    for (int channel = 0; channel< totalNumInputChannels; ++channel) {
        auto* channelData = buffer.getWritePointer(channel);
        for (int sample = 0; sample< buffer.getNumSamples(); ++sample) {
            channelData[sample] *=  gain;
        }
    }
}

//==============================================================================
bool DelayPJAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* DelayPJAudioProcessor::createEditor()
{
    return new DelayPJAudioProcessorEditor (*this);
}

//==============================================================================
void DelayPJAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    copyXmlToBinary(*apvts.copyState().createXml(), destData);
}

void DelayPJAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml.get() != nullptr && xml->hasTagName(apvts.state.getType())){
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DelayPJAudioProcessor();
}

// My code!

juce::AudioProcessorValueTreeState::ParameterLayout DelayPJAudioProcessor::createParameterLayout() {
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
