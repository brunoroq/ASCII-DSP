#include "PluginEditor.h"

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ASCIIDSPAudioProcessor();
}

ASCIIDSPAudioProcessorEditor::ASCIIDSPAudioProcessorEditor (ASCIIDSPAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (600, 400);

    startTimerHz(60);
}

ASCIIDSPAudioProcessorEditor::~ASCIIDSPAudioProcessorEditor()
{
}

void ASCIIDSPAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);

    g.setColour(juce::Colours::white);

    juce::String asciiArt =
R"(..............:::::::!!!!!!!
.........::::::*************
.....::::::********#########
...::::*****########%%%%%%%
::::*****####%%%%@@@@@@@@@@
)";

    int brightness = juce::jlimit(50, 255, (int)(currentRMS * 3000.0f));

    g.setColour(juce::Colour(brightness, brightness, brightness));

    g.setFont(16.0f);

    g.drawFittedText(
        asciiArt,
        getLocalBounds(),
        juce::Justification::centred,
        20
    );
}

void ASCIIDSPAudioProcessorEditor::resized()
{
}

void ASCIIDSPAudioProcessorEditor::timerCallback()
{
    currentRMS = audioProcessor.getRMSValue();

    repaint();
}