#include "BandColumn.h"

static juce::Colour bandColorForIndex(int idx)
{
    if (idx == 1) return PhaseManglerLookAndFeel::band1Color;
    if (idx == 2) return PhaseManglerLookAndFeel::band2Color;
    return PhaseManglerLookAndFeel::band3Color;
}

static void setupKnob(juce::Slider& s, juce::Label& l, const juce::String& name)
{
    s.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 16);
    l.setText(name, juce::dontSendNotification);
    l.setJustificationType(juce::Justification::centred);
    l.setFont(juce::Font(10.0f));
    l.setColour(juce::Label::textColourId, PhaseManglerLookAndFeel::textMuted);
}

BandColumn::BandColumn(juce::AudioProcessorValueTreeState& a, int idx)
    : apvts(a), bandIndex(idx)
{
    juce::String pre = "band" + juce::String(idx) + "_";

    enableBtn.setButtonText("Band " + juce::String(idx));
    enableBtn.setClickingTogglesState(true);
    addAndMakeVisible(enableBtn);

    setupKnob(centerKnob, centerLabel, "Center");
    setupKnob(widthKnob,  widthLabel,  "Width");
    setupKnob(rateKnob,   rateLabel,   "Rate");
    setupKnob(depthKnob,  depthLabel,  "Depth");

    for (auto* s : { &centerKnob, &widthKnob, &rateKnob, &depthKnob })
        addAndMakeVisible(s);
    for (auto* l : { &centerLabel, &widthLabel, &rateLabel, &depthLabel })
        addAndMakeVisible(l);

    enableAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvts, pre + "enable", enableBtn);
    centerAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, pre + "center", centerKnob);
    widthAttach  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, pre + "width",  widthKnob);
    rateAttach   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, pre + "rate",   rateKnob);
    depthAttach  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, pre + "depth",  depthKnob);
}

BandColumn::~BandColumn() = default;

void BandColumn::paint(juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    g.setColour(PhaseManglerLookAndFeel::surface);
    g.fillRoundedRectangle(b, 4.0f);
    g.setColour(bandColorForIndex(bandIndex).withAlpha(0.4f));
    g.drawRoundedRectangle(b.reduced(0.5f), 4.0f, 1.5f);
}

void BandColumn::resized()
{
    auto b = getLocalBounds().reduced(6);
    int btnH  = 24;
    int labelH = 14;
    int knobH  = (b.getHeight() - btnH - 8 - labelH * 4) / 4;

    enableBtn.setBounds(b.removeFromTop(btnH));
    b.removeFromTop(8);

    juce::Slider* knobs[]  = { &centerKnob, &widthKnob, &rateKnob, &depthKnob };
    juce::Label*  labels[] = { &centerLabel, &widthLabel, &rateLabel, &depthLabel };

    for (int i = 0; i < 4; ++i)
    {
        labels[i]->setBounds(b.removeFromTop(labelH));
        knobs[i]->setBounds(b.removeFromTop(knobH));
    }
}
