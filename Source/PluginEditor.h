#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "UI/LookAndFeel.h"
#include "UI/PresetPanel/PresetPanel.h"
#include "UI/FrequencyDisplay.h"
#include "UI/BandColumn.h"

class PhaseManglerEditor : public juce::AudioProcessorEditor
{
public:
    explicit PhaseManglerEditor(PhaseManglerProcessor&);
    ~PhaseManglerEditor() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    PhaseManglerProcessor& processor;

    // LookAndFeel must be declared FIRST — destroyed last
    PhaseManglerLookAndFeel laf;

    PresetPanel      presetPanel;
    FrequencyDisplay freqDisplay;
    BandColumn       band1Col, band2Col, band3Col;

    juce::Slider wetMixKnob;
    juce::Label  wetMixLabel;
    juce::Label  buildStamp;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> wetMixAttach;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PhaseManglerEditor)
};
