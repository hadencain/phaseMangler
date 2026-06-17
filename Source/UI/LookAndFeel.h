#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class PhaseManglerLookAndFeel : public juce::LookAndFeel_V4
{
public:
    inline static const juce::Colour background   { 0xff131318 };
    inline static const juce::Colour surface      { 0xff1e1e26 };
    inline static const juce::Colour border       { 0xff303040 };
    inline static const juce::Colour accent       { 0xff6e88c8 };
    inline static const juce::Colour textPrimary  { 0xffe0e0ee };
    inline static const juce::Colour textMuted    { 0xff606070 };
    inline static const juce::Colour band1Color   { 0xff5588dd };
    inline static const juce::Colour band2Color   { 0xffddaa33 };
    inline static const juce::Colour band3Color   { 0xffdd4444 };

    PhaseManglerLookAndFeel();

    void drawRotarySlider(juce::Graphics&, int x, int y, int w, int h,
                          float sliderPosProportional, float rotaryStartAngle,
                          float rotaryEndAngle, juce::Slider&) override;

    void drawButtonBackground(juce::Graphics&, juce::Button&,
                              const juce::Colour& backgroundColour,
                              bool isHighlighted, bool isDown) override;
};
