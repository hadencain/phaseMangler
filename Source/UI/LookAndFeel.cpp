#include "LookAndFeel.h"

PhaseManglerLookAndFeel::PhaseManglerLookAndFeel()
{
    setColour(juce::ResizableWindow::backgroundColourId, background);
    setColour(juce::Slider::rotarySliderFillColourId,    accent);
    setColour(juce::Slider::rotarySliderOutlineColourId, border);
    setColour(juce::Slider::thumbColourId,               textPrimary);
    setColour(juce::Slider::textBoxTextColourId,         textPrimary);
    setColour(juce::Slider::textBoxBackgroundColourId,   surface);
    setColour(juce::Slider::textBoxOutlineColourId,      border);
    setColour(juce::Label::textColourId,                 textPrimary);
    setColour(juce::TextButton::buttonColourId,          surface);
    setColour(juce::TextButton::textColourOnId,          textPrimary);
    setColour(juce::TextButton::textColourOffId,         textMuted);
    setColour(juce::ComboBox::backgroundColourId,        surface);
    setColour(juce::ComboBox::textColourId,              textPrimary);
    setColour(juce::ComboBox::outlineColourId,           border);
}

void PhaseManglerLookAndFeel::drawRotarySlider(
    juce::Graphics& g, int x, int y, int w, int h,
    float pos, float startAngle, float endAngle, juce::Slider&)
{
    float radius = juce::jmin(w, h) * 0.5f - 4.0f;
    float cx = x + w * 0.5f, cy = y + h * 0.5f;

    juce::Path track;
    track.addCentredArc(cx, cy, radius, radius, 0.0f, startAngle, endAngle, true);
    g.setColour(border);
    g.strokePath(track, juce::PathStrokeType(2.0f));

    float angle = startAngle + pos * (endAngle - startAngle);
    juce::Path arc;
    arc.addCentredArc(cx, cy, radius, radius, 0.0f, startAngle, angle, true);
    g.setColour(accent);
    g.strokePath(arc, juce::PathStrokeType(2.5f));

    float px = cx + radius * 0.75f * std::sin(angle);
    float py = cy - radius * 0.75f * std::cos(angle);
    g.setColour(textPrimary);
    g.drawLine(cx, cy, px, py, 2.0f);
}

void PhaseManglerLookAndFeel::drawButtonBackground(
    juce::Graphics& g, juce::Button& btn,
    const juce::Colour&, bool isHighlighted, bool isDown)
{
    auto bounds = btn.getLocalBounds().toFloat().reduced(1.0f);
    g.setColour(isDown ? accent : (isHighlighted ? surface.brighter(0.1f) : surface));
    g.fillRoundedRectangle(bounds, 3.0f);
    g.setColour(border);
    g.drawRoundedRectangle(bounds, 3.0f, 1.0f);
}
