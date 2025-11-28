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
    lowCutFilter.setType(juce::dsp::StateVariableTPTFilterType::highpass);
    highCutFilter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
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
    double numSamples = ( Parameters::maxDelayTime / 1000.0 ) * sampleRate;
    int maxDelayInSamples = int(std::ceil(numSamples));
    delayLineL.setMaximunDelayInSamples(maxDelayInSamples);
    delayLineR.setMaximunDelayInSamples(maxDelayInSamples);
    delayLineL.reset();
    delayLineR.reset();
    feedbackL = 0.0f;
    feedbackR = 0.0f;
    lowCutFilter.prepare(spec);
    lowCutFilter.reset();
    highCutFilter.prepare(spec);
    highCutFilter.reset();
    lastLowCut = -1.0f;
    lastHighCut = -1.0f;
    tempo.reset();
    levelL.reset();
    levelR.reset();
//    delayInSamples = 0.0f;
//    targetDelay = 0.0f;
//    xfade = 0.0f;
//    xfadeInc = static_cast<float>( 1.0f /(0.05 * sampleRate)); // 50 ms
    
    delayInSamples = 0.0f;
    targetDelay = 0.0f;
    
    fade = 1.0f;
    fadeTarget = 1.0f;
    
    coeff = 1.0f - std::exp(-1.0f / (0.05f * float(sampleRate)));
    
    wait = 0.0f;
    waitInc = 1.0f / (0.3f * float(sampleRate)); // 300 ms
    
}

void DelayPJAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool DelayPJAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto mono = juce::AudioChannelSet::mono();
    const auto stereo = juce::AudioChannelSet::stereo();
    const auto mainIn = layouts.getMainInputChannelSet();
    const auto mainOut = layouts.getMainOutputChannelSet();
    if (mainIn == mono && mainOut == mono) { return true; }
    if (mainIn == mono && mainOut == stereo) { return true; }
    if (mainIn == stereo && mainOut == stereo) { return true; }
    return false;
}

void DelayPJAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, [[maybe_unused]] juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    params.update();
    if (params.bypassed) { return; }
    tempo.update(getPlayHead());
    float syncedTime = float(tempo.getMillisecondsForNoteLength(params.delayNote));
    if (syncedTime > Parameters::maxDelayTime) {
        syncedTime = Parameters::maxDelayTime;
    }
    float sampleRate = float(getSampleRate());
    
    auto mainInput = getBusBuffer(buffer, true, 0);
    auto mainInputChannels = mainInput.getNumChannels();
    auto isMainInputStereo = mainInputChannels > 1;
    const float* inputDataL = mainInput.getReadPointer(0);
    const float* inputDataR = mainInput.getReadPointer(isMainInputStereo ? 1 : 0);
    
    auto mainOutput = getBusBuffer(buffer, false, 0);
    auto mainOutputChannels = mainOutput.getNumChannels();
    auto isMainOutputStereo = mainOutputChannels > 1;
    float* outputDataL = mainOutput.getWritePointer(0);
    float * outputDataR = mainOutput.getWritePointer(isMainOutputStereo ? 1 : 0);
    if (isMainOutputStereo) {
        float maxL = 0.0f;
        float maxR = 0.0f;
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            params.smoothen();
            float delayTime = params.tempoSync ? syncedTime : params.delayTime;
            float newTargetDelay = delayTime / 1000.0f * sampleRate;
            if (newTargetDelay != targetDelay) {
                targetDelay = newTargetDelay;
                if(delayInSamples == 0.0f) {
                    delayInSamples = targetDelay;
                }
                else {
                    wait = waitInc;
                    fadeTarget = 0.0f;
                }
            }
//            if (xfade == 0.0f) {
//                float delayTime = params.tempoSync ? syncedTime : params.delayTime;
//                targetDelay = delayTime / 1000.0f * sampleRate;
//                if (delayInSamples == 0.0f) {
//                    delayInSamples = targetDelay;
//                } else if (targetDelay != delayInSamples) {
//                    xfade = xfadeInc;
//                }
//            }
            
            if (params.lowCut != lastLowCut) {
                lowCutFilter.setCutoffFrequency(params.lowCut);
                lastLowCut = params.lowCut;
            }
            if (params.highCut != lastHighCut) {
                highCutFilter.setCutoffFrequency(params.highCut);
                lastHighCut = params.highCut;
            }
            
            float dryL = inputDataL[sample];
            float dryR = inputDataR[sample];
            
            // convert stereo to mono
            float mono = (dryL + dryR) * 0.5f;
            
            delayLineL.write(mono * params.panL + feedbackR);
            delayLineR.write(mono * params.panR + feedbackL);
            
            float wetL = delayLineL.read(delayInSamples);
            float wetR = delayLineR.read(delayInSamples);
            
            fade += (fadeTarget - fade) * coeff;
            
            wetL *= fade;
            wetR *= fade;
            
            if(wait > 0.0f) {
                wait += waitInc;
                if(wait >= 1.0f) {
                    delayInSamples = targetDelay;
                    wait = 0.0f;
                    fadeTarget = 1.0f; //fade in
                }
            }
            
//            if (xfade > 0.0f) {
//                float newL = delayLineL.read(targetDelay);
//                float newR = delayLineR.read(targetDelay);
//                wetL = (1.0f - xfade) * wetL + xfade * newL;
//                wetR = (1.0f - xfade) * wetR + xfade * newR;
//                
//                xfade += xfadeInc;
//                if (xfade >= 1.0f) {
//                    delayInSamples = targetDelay;
//                    xfade = 0.0f;
//                }
//            }
            
            feedbackL = wetL * params.feedback;
            feedbackL = lowCutFilter.processSample(0, feedbackL);
            feedbackL = highCutFilter.processSample(0, feedbackL);
            
            feedbackR = wetR * params.feedback;
            feedbackR = lowCutFilter.processSample(1, feedbackR);
            feedbackR = highCutFilter.processSample(1, feedbackR);
            
            // applying linera interpolation: a*(1-c) + b*c, where c[0-1.0]
            float mixL = dryL * (1.0f - params.mix) + wetL * params.mix;
            float mixR = dryR * (1.0f - params.mix) + wetR * params.mix;
            //        float mixL = dryL + wetL * params.mix;
            //        float mixR = dryR + wetR * params.mix;
            
            float outL = mixL * params.gain;
            float outR = mixR * params.gain;
            
            if (params.bypassed) {
                outL = dryL;
                outR = dryR;
            }
            
            outputDataL[sample] = outL;
            outputDataR[sample] = outR;
            
            maxL = std::max(maxL, std::abs(outL));
            maxR = std::max(maxR, std::abs(outR));
        }
        levelL.updateIfGreater(maxL);
        levelR.updateIfGreater(maxR);
        } else {
            float maxL = 0.0f;
            for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
                params.smoothen();
                float delayTime = params.tempoSync ? syncedTime : params.delayTime;
                float newTargetDelay = delayTime / 1000.0f * sampleRate;
                if (newTargetDelay != targetDelay) {
                    targetDelay = newTargetDelay;
                    if(delayInSamples == 0.0f) {
                        delayInSamples = targetDelay;
                    }
                    else {
                        wait = waitInc;
                        fadeTarget = 0.0f;
                    }
                }
//                if (xfade == 0.0f) {
//                    float delayTime = params.tempoSync ? syncedTime : params.delayTime;
//                    targetDelay = delayTime / 1000.0f * sampleRate;
//                    if (delayInSamples == 0.0f) {
//                        delayInSamples = targetDelay;
//                    } else if (targetDelay != delayInSamples) {
//                        xfade = xfadeInc;
//                    }
//                }
                
                if (params.lowCut != lastLowCut) {
                    lowCutFilter.setCutoffFrequency(params.lowCut);
                    lastLowCut = params.lowCut;
                }
                if (params.highCut != lastHighCut) {
                    highCutFilter.setCutoffFrequency(params.highCut);
                    lastHighCut = params.highCut;
                }
                
                float dry = inputDataL[sample];
                delayLineL.write(dry + feedbackL);
                float wet = delayLineL.read(delayInSamples);
                
                fade += (fadeTarget - fade) * coeff;
                wet *= fade;
                
                if(wait > 0.0f) {
                    wait += waitInc;
                    if(wait >= 1.0f) {
                        delayInSamples = targetDelay;
                        wait = 0.0f;
                        fadeTarget = 1.0f; //fade in
                    }
                }
                
//                if (xfade > 0.0f) {
//                    float newL = delayLineL.read(targetDelay);
//                    wet = (1.0f - xfade) * wet + xfade * newL;
//                    xfade += xfadeInc;
//                    if(xfade >= 1.0f) {
//                        delayInSamples = targetDelay;
//                        xfade = 0.0f;
//                    }
//                }
                
                feedbackL = wet * params.feedback;
                feedbackL = lowCutFilter.processSample(0, feedbackL);
                feedbackL = highCutFilter.processSample(0, feedbackL);
                float mix = dry + wet * params.mix;
                float outL = mix * params.gain;
                if (params.bypassed) {
                    outL = dry;
                }
                outputDataL[sample] = outL;
                maxL = std::max(maxL, std::abs(outL));
            }
            levelL.updateIfGreater(maxL);
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

juce::AudioProcessorParameter* DelayPJAudioProcessor::getBypassParameter() const
{
    return params.bypassParam;
}
