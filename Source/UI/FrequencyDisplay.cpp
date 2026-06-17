#include "FrequencyDisplay.h"
#include <cmath>

FrequencyDisplay::FrequencyDisplay(juce::AudioProcessorValueTreeState& a)
    : apvts(a)
{
    for (int b = 1; b <= 3; ++b)
    {
        juce::String pre = "band" + juce::String(b) + "_";
        apvts.addParameterListener(pre + "enable", this);
        apvts.addParameterListener(pre + "center", this);
        apvts.addParameterListener(pre + "width",  this);
    }
}

FrequencyDisplay::~FrequencyDisplay()
{
    for (int b = 1; b <= 3; ++b)
    {
        juce::String pre = "band" + juce::String(b) + "_";
        apvts.removeParameterListener(pre + "enable", this);
        apvts.removeParameterListener(pre + "center", this);
        apvts.removeParameterListener(pre + "width",  this);
    }
}

void FrequencyDisplay::parameterChanged(const juce::String&, float)
{
    repaint();
}

float FrequencyDisplay::freqToX(float hz, float width) const noexcept
{
    const float logMin = std::log10(20.0f);
    const float logMax = std::log10(20000.0f);
    float logHz = std::log10(juce::jlimit(20.0f, 20000.0f, hz));
    return (logHz - logMin) / (logMax - logMin) * width;
}

void FrequencyDisplay::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    float w = bounds.getWidth();
    float h = bounds.getHeight();

    g.setColour(PhaseManglerLookAndFeel::surface);
    g.fillRoundedRectangle(bounds, 4.0f);
    g.setColour(PhaseManglerLookAndFeel::border);
    g.drawRoundedRectangle(bounds.reduced(0.5f), 4.0f, 1.0f);

    g.setColour(PhaseManglerLookAndFeel::border.withAlpha(0.5f));
    for (float freq : { 50.0f, 100.0f, 200.0f, 500.0f, 1000.0f, 2000.0f, 5000.0f, 10000.0f })
    {
        float x = freqToX(freq, w);
        g.drawVerticalLine(int(x), 2.0f, h - 2.0f);
    }

    for (int b = 0; b < NUM_BANDS; ++b)
    {
        juce::String pre = "band" + juce::String(b + 1) + "_";
        float enabled = apvts.getRawParameterValue(pre + "enable")->load();
        if (enabled < 0.5f) continue;

        float center = apvts.getRawParameterValue(pre + "center")->load();
        float width  = apvts.getRawParameterValue(pre + "width")->load();
        float halfFactor = std::pow(2.0f, width * 0.5f);
        float loHz = center / halfFactor;
        float hiHz = center * halfFactor;

        float x0 = freqToX(loHz, w);
        float x1 = freqToX(hiHz, w);

        juce::Colour c = bandColors[b];
        g.setColour(c.withAlpha(0.25f));
        g.fillRect(x0, 2.0f, x1 - x0, h - 4.0f);
        g.setColour(c.withAlpha(0.8f));
        g.drawVerticalLine(int((x0 + x1) * 0.5f), 2.0f, h - 2.0f);
    }

    g.setColour(PhaseManglerLookAndFeel::textMuted);
    g.setFont(9.0f);
    for (auto& pair : std::initializer_list<std::pair<float, const char*>>{
        {100.0f, "100"}, {1000.0f, "1k"}, {10000.0f, "10k"}})
    {
        float x = freqToX(pair.first, w);
        g.drawText(pair.second, int(x) - 10, int(h) - 12, 20, 12, juce::Justification::centred);
    }
}
