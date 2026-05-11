#include "PluginEditor.h"
#include "BinaryData.h"

ASCIIDSPAudioProcessorEditor::ASCIIDSPAudioProcessorEditor (ASCIIDSPAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (800, 600);

    characterImage = juce::ImageFileFormat::loadFrom(
        BinaryData::character_png,
        BinaryData::character_pngSize
    );

    startTimerHz(60);
}

ASCIIDSPAudioProcessorEditor::~ASCIIDSPAudioProcessorEditor()
{
}

void ASCIIDSPAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);

    if (characterImage.isValid())
    {
        float scale = 1.0f + (currentRMS * 0.5f);

        int width = (int)(characterImage.getWidth() * scale);
        int height = (int)(characterImage.getHeight() * scale);

        int x = (getWidth() - width) / 2;
        int y = (getHeight() - height) / 2;

        g.setOpacity(0.5f + currentRMS);

        g.drawImage(
            characterImage,
            x,
            y,
            width,
            height,
            0,
            0,
            characterImage.getWidth(),
            characterImage.getHeight()
        );
    }

    g.setColour(juce::Colours::white);

    g.drawText(
        "ASCII DSP",
        20,
        20,
        200,
        40,
        juce::Justification::left
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
