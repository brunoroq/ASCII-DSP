#include "PluginProcessor.h"
#include "PluginEditor.h"

ASCIIDSPAudioProcessor::ASCIIDSPAudioProcessor()
     : AudioProcessor (BusesProperties()
                     .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                     .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
}

ASCIIDSPAudioProcessor::~ASCIIDSPAudioProcessor()
{
}

const juce::String ASCIIDSPAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ASCIIDSPAudioProcessor::acceptsMidi() const { return false; }
bool ASCIIDSPAudioProcessor::producesMidi() const { return false; }
bool ASCIIDSPAudioProcessor::isMidiEffect() const { return false; }
double ASCIIDSPAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int ASCIIDSPAudioProcessor::getNumPrograms() { return 1; }
int ASCIIDSPAudioProcessor::getCurrentProgram() { return 0; }
void ASCIIDSPAudioProcessor::setCurrentProgram (int index) {}
const juce::String ASCIIDSPAudioProcessor::getProgramName (int index) { return {}; }
void ASCIIDSPAudioProcessor::changeProgramName (int index, const juce::String& newName) {}

void ASCIIDSPAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
}

void ASCIIDSPAudioProcessor::releaseResources()
{
}

bool ASCIIDSPAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return true;
}

void ASCIIDSPAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                           juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    float sum = 0.0f;
    int numSamples = buffer.getNumSamples();

    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto* data = buffer.getReadPointer(channel);

        for (int i = 0; i < numSamples; ++i)
        {
            sum += data[i] * data[i];
        }
    }

    rmsValue = std::sqrt(sum / (buffer.getNumChannels() * numSamples));
}

bool ASCIIDSPAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* ASCIIDSPAudioProcessor::createEditor()
{
    return new ASCIIDSPAudioProcessorEditor (*this);
}

void ASCIIDSPAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
}

void ASCIIDSPAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
}

float ASCIIDSPAudioProcessor::getRMSValue() const
{
    return rmsValue;
}