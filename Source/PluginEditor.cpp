#include "PluginEditor.h"
#include "BinaryData.h"
using namespace juce;
void ASCIIDSPAudioProcessorEditor::generateAsciiVariants()
{
    asciiNormal.clear();
    asciiInvert.clear();
    asciiHeavy.clear();

    if (! characterImage.isValid())
        return;

    juce::String normalChars = " .:-=+*#%@";
    juce::String heavyChars  = "  ..::--==++**##%%@@";

    int cellSize = 8;

    for (int y = 0; y < characterImage.getHeight(); y += cellSize)
    {
        String normalLine;
        String invertLine;
        String heavyLine;

        for (int x = 0; x < characterImage.getWidth(); x += cellSize)
        {
            auto pixel = characterImage.getPixelAt(x, y);

            float brightness =
                (pixel.getFloatRed()
                + pixel.getFloatGreen()
                + pixel.getFloatBlue()) / 3.0f;

            brightness = juce::jlimit(0.0f, 1.0f, brightness);

            int normalIndex = (int)(brightness * (normalChars.length() - 1));
            int invertIndex = (int)((1.0f - brightness) * (normalChars.length() - 1));
            int heavyIndex  = (int)(brightness * (heavyChars.length() - 1));

            normalLine += normalChars[normalIndex];
            invertLine += normalChars[invertIndex];
            heavyLine  += heavyChars[heavyIndex];
        }

        asciiNormal.add(normalLine);
        asciiInvert.add(invertLine);
        asciiHeavy.add(heavyLine);
    }
}

ASCIIDSPAudioProcessorEditor::ASCIIDSPAudioProcessorEditor (ASCIIDSPAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (800, 600);

    characterImage = ImageFileFormat::loadFrom(
        BinaryData::character_png,
        BinaryData::character_pngSize
    );
    generateAsciiVariants();
    startTimerHz(30);
}

ASCIIDSPAudioProcessorEditor::~ASCIIDSPAudioProcessorEditor()
{
}

void ASCIIDSPAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll(juce::Colours::black);

    float audio = std::sqrt(currentRMS) * 2.5f;
    audio = juce::jlimit(0.0f, 1.0f, audio);

    int squareSize = juce::jmin(getWidth(), getHeight()) - 80;
    juce::Rectangle<int> asciiArea(
        (getWidth() - squareSize) / 2,
        (getHeight() - squareSize) / 2,
        squareSize,
        squareSize
    );

    const auto& lines =
        audio > 0.75f ? asciiHeavy :
                        asciiNormal;

    int lineHeight = 16;
    int charWidth = 6;

    int asciiWidth = lines[0].length() * charWidth;
    int asciiHeight = lines.size() * lineHeight;

    int x = asciiArea.getX() + (asciiArea.getWidth() - asciiWidth) / 2;
    int y = asciiArea.getY() + (asciiArea.getHeight() - asciiHeight) / 2;

    int brightness = juce::jlimit(35, 190, (int)(55 + audio * 135));
    g.setColour(juce::Colour(brightness, brightness, brightness));
    g.setFont(juce::FontOptions(14.0f));

    for (int i = 0; i < lines.size(); ++i)
    {
        g.drawText(
            lines[i],
            x,
            y + i * lineHeight,
            getWidth() - 120,
            lineHeight,
            juce::Justification::left
        );
    }
}

void ASCIIDSPAudioProcessorEditor::resized()
{
}

void ASCIIDSPAudioProcessorEditor::timerCallback()
{
    currentRMS = audioProcessor.getRMSValue();

    repaint();
}
