#pragma once

#include "PluginProcessor.h"

class ASCIIDSPAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                      private juce::Timer
{
public:
    explicit ASCIIDSPAudioProcessorEditor (ASCIIDSPAudioProcessor& processor);
    ~ASCIIDSPAudioProcessorEditor() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    void timerCallback() override;

    ASCIIDSPAudioProcessor& audioProcessor;

    float currentRMS = 0.0f;
    juce::StringArray asciiNormal;
    juce::StringArray asciiInvert;
    juce::StringArray asciiHeavy;

    void generateAsciiVariants();
    juce::Image characterImage;
    

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ASCIIDSPAudioProcessorEditor)
};