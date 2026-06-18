#include "PluginEditor.h"

PhaseManglerEditor::PhaseManglerEditor(PhaseManglerProcessor& p)
    : AudioProcessorEditor(p), processor(p),
      presetPanel(p.apvts, "PhaseMangler"),
      freqDisplay(p.apvts),
      band1Col(p.apvts, 1),
      band2Col(p.apvts, 2),
      band3Col(p.apvts, 3)
{
    setLookAndFeel(&laf);
    setSize(600, 440);

    addAndMakeVisible(presetPanel);
    addAndMakeVisible(freqDisplay);
    addAndMakeVisible(band1Col);
    addAndMakeVisible(band2Col);
    addAndMakeVisible(band3Col);

    wetMixKnob.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    wetMixKnob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 52, 20);
    addAndMakeVisible(wetMixKnob);

    wetMixLabel.setText("Wet", juce::dontSendNotification);
    wetMixLabel.setJustificationType(juce::Justification::centred);
    wetMixLabel.setFont(juce::Font(11.0f));
    wetMixLabel.setColour(juce::Label::textColourId, PhaseManglerLookAndFeel::textPrimary);
    addAndMakeVisible(wetMixLabel);

    buildStamp.setText(juce::String(__DATE__) + " " + CMAKE_BUILD_TYPE,
                       juce::dontSendNotification);
    buildStamp.setFont(juce::Font(9.0f));
    buildStamp.setColour(juce::Label::textColourId, PhaseManglerLookAndFeel::textMuted);
    buildStamp.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(buildStamp);

    wetMixAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        p.apvts, "wetMix", wetMixKnob);
}

void PhaseManglerEditor::paint(juce::Graphics& g)
{
    g.fillAll(PhaseManglerLookAndFeel::background);
}

void PhaseManglerEditor::resized()
{
    auto b = getLocalBounds();

    presetPanel.setBounds(b.removeFromTop(30));
    buildStamp.setBounds(b.removeFromBottom(14));
    freqDisplay.setBounds(b.removeFromTop(120).reduced(4, 2));

    int wetW = 60;
    auto wetArea = b.removeFromRight(wetW);
    int colW = b.getWidth() / 3;

    band1Col.setBounds(b.removeFromLeft(colW).reduced(3));
    band2Col.setBounds(b.removeFromLeft(colW).reduced(3));
    band3Col.setBounds(b.reduced(3));

    int knobH = 60;
    int labelH = 14;
    auto wetKnobArea = wetArea.withSizeKeepingCentre(wetW - 8, knobH + labelH);
    wetMixLabel.setBounds(wetKnobArea.removeFromTop(labelH));
    wetMixKnob.setBounds(wetKnobArea);
}
