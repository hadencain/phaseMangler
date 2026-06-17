#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "LookAndFeel.h"
#include "../DSP/BandDescriptor.h"

class FrequencyDisplay : public juce::Component,
                         public juce::AudioProcessorValueTreeState::Listener
{
public:
    FrequencyDisplay(juce::AudioProcessorValueTreeState& apvts);
    ~FrequencyDisplay() override;

    void paint(juce::Graphics& g) override;
    void parameterChanged(const juce::String& paramID, float) override;

private:
    juce::AudioProcessorValueTreeState& apvts;

    static constexpr int NUM_BANDS = 3;
    inline static const juce::Colour bandColors[NUM_BANDS] = {
        PhaseManglerLookAndFeel::band1Color,
        PhaseManglerLookAndFeel::band2Color,
        PhaseManglerLookAndFeel::band3Color
    };

    float freqToX(float hz, float width) const noexcept;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FrequencyDisplay)
};
