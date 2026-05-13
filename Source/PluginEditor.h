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
    static constexpr int ASCII_COLUMNS = 90;
    static constexpr int ASCII_ROWS = 55;
    static constexpr int MARGIN = 40;
    static constexpr int PADDING = 10;
    static constexpr float LINE_HEIGHT_FACTOR = 1.2f;
    static constexpr float FONT_SIZE_FACTOR = 0.9f;
    static constexpr float HEAVY_THRESHOLD = 0.75f;

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