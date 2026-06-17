#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "DSP/BandDescriptor.h"
#include "DSP/PhaseScrambler.h"

class PhaseManglerProcessor : public juce::AudioProcessor
{
public:
    PhaseManglerProcessor();
    ~PhaseManglerProcessor() override = default;

    void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) override;
    void releaseResources() override {}
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "PhaseMangler"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getLatencySamples() const { return scrambler.getLatencySamples(); }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    PhaseScrambler scrambler;
    juce::SmoothedValue<float> smoothWet;

    std::atomic<float>* pWetMix = nullptr;
    struct BandParams {
        std::atomic<float>* enable = nullptr;
        std::atomic<float>* center = nullptr;
        std::atomic<float>* width  = nullptr;
        std::atomic<float>* rate   = nullptr;
        std::atomic<float>* depth  = nullptr;
    };
    BandParams bandParams[3];

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PhaseManglerProcessor)
};
