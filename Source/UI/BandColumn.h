#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "LookAndFeel.h"

class BandColumn : public juce::Component
{
public:
    BandColumn(juce::AudioProcessorValueTreeState& apvts, int bandIndex);
    ~BandColumn() override;
    void resized() override;
    void paint(juce::Graphics& g) override;

private:
    juce::AudioProcessorValueTreeState& apvts;
    int bandIndex; // 1-based

    juce::TextButton enableBtn;
    juce::Slider centerKnob, widthKnob, rateKnob, depthKnob;
    juce::Label  centerLabel, widthLabel, rateLabel, depthLabel;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>  enableAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>  centerAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>  widthAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>  rateAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>  depthAttach;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BandColumn)
};
