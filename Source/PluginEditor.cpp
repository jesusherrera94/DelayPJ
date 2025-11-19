/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DelayPJAudioProcessorEditor::DelayPJAudioProcessorEditor (DelayPJAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    delayGroup.setText("Delay");
    delayGroup.setTextLabelPosition(juce::Justification::horizontallyCentred);
    delayGroup.addAndMakeVisible(delayTimeKnob);
    delayGroup.addChildComponent(delayNoteKnob);
    tempoSyncButton.setButtonText("Sync");
    tempoSyncButton.setClickingTogglesState(true);
    tempoSyncButton.setBounds(0,0,70,27);
    tempoSyncButton.setLookAndFeel(ButtonLookAndFeel::get());
    delayGroup.addAndMakeVisible(tempoSyncButton);
    addAndMakeVisible(delayGroup);
    
    feedbackGroup.setText("Feedback");
    feedbackGroup.setTextLabelPosition(juce::Justification::horizontallyCentred);
    feedbackGroup.addAndMakeVisible(feedbackKnob);
    feedbackGroup.addAndMakeVisible(stereoKnob);
    feedbackGroup.addAndMakeVisible(lowCutKnob);
    feedbackGroup.addAndMakeVisible(highCutKnob);
    addAndMakeVisible(feedbackGroup);
    
    outputGroup.setText("Output");
    outputGroup.setTextLabelPosition(juce::Justification::horizontallyCentred);
    outputGroup.addAndMakeVisible(gainKnob);
    outputGroup.addAndMakeVisible(mixKnob);
    addAndMakeVisible(outputGroup);
    setSize (500, 330);
    setLookAndFeel(&mainLF);
    updateDelayKnobs(audioProcessor.params.tempoSyncParam->get());
    audioProcessor.params.tempoSyncParam->addListener(this);
}

DelayPJAudioProcessorEditor::~DelayPJAudioProcessorEditor()
{
    audioProcessor.params.tempoSyncParam->removeListener(this);
    setLookAndFeel(nullptr);
}

//==============================================================================
void DelayPJAudioProcessorEditor::paint (juce::Graphics& g)
{
    // background image
    auto noise = juce::ImageCache::getFromMemory(BinaryData::Noise_png, BinaryData::Noise_pngSize);
    auto fillType = juce::FillType(noise, juce::AffineTransform::scale(0.5f));
    g.setFillType(fillType);
    g.fillRect(getLocalBounds());
    // logo
    auto rect = getLocalBounds().withHeight(40);
    g.setColour(Colors::header);
    g.fillRect(rect);
    auto image = juce::ImageCache::getFromMemory(BinaryData::Logo_png, BinaryData::Logo_pngSize);
    int destWidth = image.getWidth() / 2;
    int destHeight = image.getHeight() / 2;
    g.drawImage(image, getWidth() / 2 - destWidth / 2, 0, destWidth, destHeight, 0, 0, image.getWidth(), image.getHeight());
}

void DelayPJAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    
    int y = 50;
    int height = bounds.getHeight() - 60;
    
    // Position the groups
    delayGroup.setBounds(10, y, 110, height);
    outputGroup.setBounds(bounds.getWidth() - 160, y, 150, height);
    feedbackGroup.setBounds(delayGroup.getRight() + 10, y, outputGroup.getX() - delayGroup.getRight() - 20, height);
    feedbackKnob.setTopLeftPosition(20, 20);
    stereoKnob.setTopLeftPosition(feedbackKnob.getRight() + 20, 20);
    lowCutKnob.setTopLeftPosition(feedbackKnob.getX(), feedbackKnob.getBottom() + 10);
    highCutKnob.setTopLeftPosition(lowCutKnob.getRight() + 20, lowCutKnob.getY());
    // Position the knobs inside the groups
    delayTimeKnob.setTopLeftPosition(20, 20);
    tempoSyncButton.setTopLeftPosition(20, delayTimeKnob.getBottom() + 10);
    // place the knob in top of delay time knob
    delayNoteKnob.setTopLeftPosition(delayTimeKnob.getX(), delayTimeKnob.getY());
    mixKnob.setTopLeftPosition(20, 20);
    gainKnob.setTopLeftPosition(mixKnob.getX(), mixKnob.getBottom() + 20);
    
}

void DelayPJAudioProcessorEditor::parameterValueChanged(int, float value) {
    // if it is executing on the message thread(UI), update directly
    if (juce::MessageManager::getInstance()->isThisTheMessageThread()){
        updateDelayKnobs(value != 0.0f);
    } else {
        // if it is called directly from the audio thread, defer the update to the message thread
        juce::MessageManager::callAsync([this, value]{
            updateDelayKnobs(value != 0.0f);
        });
    }
}

void DelayPJAudioProcessorEditor::updateDelayKnobs(bool tempoSyncActive) {
    delayTimeKnob.setVisible(!tempoSyncActive);
    delayNoteKnob.setVisible(tempoSyncActive);
}
