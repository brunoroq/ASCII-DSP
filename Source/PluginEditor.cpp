#include "PluginEditor.h"
#include "BinaryData.h"
using namespace juce;

namespace
{
    // Small ordered-dither matrix to keep the pattern stable per cell.
    constexpr int bayer8[8][8] =
    {
        {  0, 48, 12, 60,  3, 51, 15, 63 },
        { 32, 16, 44, 28, 35, 19, 47, 31 },
        {  8, 56,  4, 52, 11, 59,  7, 55 },
        { 40, 24, 36, 20, 43, 27, 39, 23 },
        {  2, 50, 14, 62,  1, 49, 13, 61 },
        { 34, 18, 46, 30, 33, 17, 45, 29 },
        { 10, 58,  6, 54,  9, 57,  5, 53 },
        { 42, 26, 38, 22, 41, 25, 37, 21 }
    };

    float getOrderedDither (int x, int y)
    {
        // Range: roughly [-0.5, 0.5]
        return ((float) bayer8[y & 7][x & 7] + 0.5f) / 64.0f - 0.5f;
    }
}

void ASCIIDSPAudioProcessorEditor::generateAsciiVariants()
{
    asciiNormal.clear();
    asciiInvert.clear();
    asciiHeavy.clear();

    if (!characterImage.isValid())
        return;

    // Character sets for brightness mapping
    juce::String normalChars = " .:-=+*#%@";
    juce::String heavyChars  = "  ..::--==++**##%%@@";

    // Get original image dimensions
    int imgWidth = characterImage.getWidth();
    int imgHeight = characterImage.getHeight();

    // Calculate scale to fit image into the rectangular ASCII grid with letterbox
    float scale = juce::jmin((float)ASCII_COLUMNS / imgWidth, (float)ASCII_ROWS / imgHeight);

    // Calculate scaled dimensions and offset for centering (letterbox)
    int scaledWidth = (int)(imgWidth * scale);
    int scaledHeight = (int)(imgHeight * scale);
    int offsetX = (ASCII_COLUMNS - scaledWidth) / 2;
    int offsetY = (ASCII_ROWS - scaledHeight) / 2;

    // Generate the rectangular ASCII grid
    for (int gridY = 0; gridY < ASCII_ROWS; ++gridY)
    {
        String normalLine;
        String invertLine;
        String heavyLine;

        for (int gridX = 0; gridX < ASCII_COLUMNS; ++gridX)
        {
            float brightness = 0.0f;

            // Sample image or use white for letterbox areas
            if (gridX >= offsetX && gridX < offsetX + scaledWidth &&
                gridY >= offsetY && gridY < offsetY + scaledHeight)
            {
                // Map grid position back to original image coordinates
                int imgX = (int)((gridX - offsetX) / scale);
                int imgY = (int)((gridY - offsetY) / scale);

                // Clamp to image bounds
                imgX = juce::jlimit(0, imgWidth - 1, imgX);
                imgY = juce::jlimit(0, imgHeight - 1, imgY);

                // Sample pixel and calculate brightness
                auto pixel = characterImage.getPixelAt(imgX, imgY);
                brightness = (pixel.getFloatRed() + pixel.getFloatGreen() + pixel.getFloatBlue()) / 3.0f;
            }
            else
            {
                // Letterbox areas are white (brightness = 1.0)
                brightness = 1.0f;
            }

            brightness = juce::jlimit(0.0f, 1.0f, brightness);

            // Stable dithering keeps the ASCII from forming hard horizontal bands.
            const float ditherStrength = 0.12f;
            brightness = juce::jlimit(0.0f, 1.0f,
                                      brightness + (getOrderedDither (gridX, gridY) * ditherStrength));

            // Map brightness to character indices
            int normalIndex = (int)(brightness * (normalChars.length() - 1));
            int invertIndex = (int)((1.0f - brightness) * (normalChars.length() - 1));
            int heavyIndex = (int)(brightness * (heavyChars.length() - 1));

            normalLine += normalChars[normalIndex];
            invertLine += normalChars[invertIndex];
            heavyLine += heavyChars[heavyIndex];
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

    // Load image from binary data and generate precalculated ASCII variants
    characterImage = ImageFileFormat::loadFrom(
        BinaryData::character_png,
        BinaryData::character_pngSize
    );

    generateAsciiVariants();

    // Start timer for RMS updates and repainting (30 Hz)
    startTimerHz(30);
}

ASCIIDSPAudioProcessorEditor::~ASCIIDSPAudioProcessorEditor()
{
}

void ASCIIDSPAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll(juce::Colours::black);

    // Normalize audio level and map to 0-1 range
    float audio = std::sqrt(currentRMS) * 2.5f;
    audio = juce::jlimit(0.0f, 1.0f, audio);

    // Calculate centered square canvas
    int squareSize = juce::jmin(getWidth(), getHeight()) - (2 * MARGIN);
    squareSize = juce::jmax(squareSize, 100); // Ensure minimum size

    int squareX = (getWidth() - squareSize) / 2;
    int squareY = (getHeight() - squareSize) / 2;

    juce::Rectangle<int> asciiArea(squareX, squareY, squareSize, squareSize);

    // Select ASCII variant based on audio level
    const auto& lines = audio > HEAVY_THRESHOLD ? asciiHeavy : asciiNormal;

    // Validate ASCII lines exist
    if (lines.isEmpty())
        return;

    // Fit the rectangular text block into the square area.
    // Rows define vertical spacing; columns define the visual width.
    int lineHeight = juce::jmax(1, (asciiArea.getHeight() - (2 * PADDING)) / ASCII_ROWS);
    float fontSize = lineHeight * FONT_SIZE_FACTOR;

    // Estimate the drawn width of one monospace character so the block can be centered.
    int charWidth = juce::jmax(1, (int) std::round(fontSize * 0.60f));
    int asciiWidth = ASCII_COLUMNS * charWidth;
    int asciiHeight = ASCII_ROWS * lineHeight;

    int startX = asciiArea.getX() + (asciiArea.getWidth() - asciiWidth) / 2;
    int startY = asciiArea.getY() + (asciiArea.getHeight() - asciiHeight) / 2;

    // Calculate color brightness based on audio level
    int colorBrightness = juce::jlimit(35, 190, (int)(55 + audio * 135));
    g.setColour(juce::Colour(colorBrightness, colorBrightness, colorBrightness));

    // Set monospaced font explicitly
    g.setFont(juce::FontOptions(fontSize).withName("Courier New"));

    // Draw each line of ASCII art
    for (int i = 0; i < lines.size(); ++i)
    {
        int yPos = startY + (i * lineHeight);

        g.drawText(
            lines[i],
            startX,
            yPos,
            asciiWidth,
            lineHeight,
            juce::Justification::centred
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
