/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ProtectYourEars.h"

//==============================================================================
DelayPJAudioProcessor::DelayPJAudioProcessor() :
    juce::AudioProcessor(
                         BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)
                         ),
    params(apvts)
{
    // Do nothing!
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
    params.prepareToPlay(sampleRate);
    params.reset();
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = juce::uint32(samplesPerBlock);
    spec.numChannels = 2;
    delayLine.prepare(spec);
    double numSamples = ( Parameters::maxDelayTime / 1000.0 ) * sampleRate;
    int maxDelayInSamples = int(std::ceil(numSamples));
    delayLine.setMaximumDelayInSamples(maxDelayInSamples);
    delayLine.reset();
    feedbackL = 0.0f;
    feedbackR = 0.0f;
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
    params.update();
    float sampleRate = float(getSampleRate());
    
    float* channelDataL = buffer.getWritePointer(0);
    float* channelDataR = buffer.getWritePointer(1);
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
        params.smoothen();
        float delayInSamples = (params.delayTime / 1000.0f) * sampleRate;
        delayLine.setDelay(delayInSamples);
        float dryL = channelDataL[sample];
        float dryR = channelDataR[sample];
        
        // convert stereo to mono
        float mono = (dryL + dryR) * 0.5f;
        
        delayLine.pushSample(0, mono * params.panL + feedbackR);
        delayLine.pushSample(1, mono * params.panR + feedbackL);
        
        float wetL = delayLine.popSample(0);
        float wetR = delayLine.popSample(1);
        feedbackL = wetL * params.feedback;
        feedbackR = wetR * params.feedback;
        
        // applying linera interpolation: a*(1-c) + b*c, where c[0-1.0]
        float mixL = dryL * (1.0f - params.mix) + wetL * params.mix;
        float mixR = dryR * (1.0f - params.mix) + wetR * params.mix;
//        float mixL = dryL + wetL * params.mix;
//        float mixR = dryR + wetR * params.mix;
        
        channelDataL[sample] = mixL * params.gain;
        channelDataR[sample] = mixR * params.gain;
    }
    // adding protect your ears utility to protect ears and speakers
    #if JUCE_DEBUG
        protectYourEars(buffer);
    #endif
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

