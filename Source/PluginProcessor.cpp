#include "PluginProcessor.h"
#include "PluginEditor.h"

MexoscopeAudioProcessor::MexoscopeAudioProcessor()
    : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                      .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
}

MexoscopeAudioProcessor::~MexoscopeAudioProcessor()
{
}

const juce::String MexoscopeAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MexoscopeAudioProcessor::acceptsMidi() const
{
    return false;
}

bool MexoscopeAudioProcessor::producesMidi() const
{
    return false;
}

bool MexoscopeAudioProcessor::isMidiEffect() const
{
    return false;
}

double MexoscopeAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MexoscopeAudioProcessor::getNumPrograms()
{
    return 1;
}

int MexoscopeAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MexoscopeAudioProcessor::setCurrentProgram(int)
{
}

const juce::String MexoscopeAudioProcessor::getProgramName(int)
{
    return {};
}

void MexoscopeAudioProcessor::changeProgramName(int, const juce::String&)
{
}

void MexoscopeAudioProcessor::prepareToPlay(double sampleRate, int)
{
    mexoscope.prepareToPlay(sampleRate);
}

void MexoscopeAudioProcessor::releaseResources()
{
}

bool MexoscopeAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
    &&  layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo()) {
        return false;
    }
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet()) {
        return false;
    }
    return true;
}

void MexoscopeAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i) {
        buffer.clear(i, 0, buffer.getNumSamples());
    }

    mexoscope.process(buffer);
}

bool MexoscopeAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* MexoscopeAudioProcessor::createEditor()
{
    return new MexoscopeAudioProcessorEditor(*this);
}

void MexoscopeAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    destData.setSize(mexoscope.getSaveBlockSize());
    destData.copyFrom(mexoscope.getSaveBlock(), 0, mexoscope.getSaveBlockSize());
}

void MexoscopeAudioProcessor::setStateInformation(const void* data, int)
{
    std::memcpy(mexoscope.getSaveBlock(), data, mexoscope.getSaveBlockSize());
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MexoscopeAudioProcessor();
}
