#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

class PresetPanel : public juce::Component
{
public:
    PresetPanel(juce::AudioProcessorValueTreeState& apvts, const juce::String& pluginName);
    ~PresetPanel() override = default;

    void resized() override;

private:
    void loadPreset(int index);
    void saveCurrentPreset();
    void savePresetAs();
    void doSaveAs(const juce::String& name);
    void deleteCurrentPreset();
    void refresh();
    juce::File presetDirectory() const;
    juce::String currentName() const;

    juce::AudioProcessorValueTreeState& apvts;
    juce::String pluginName;
    juce::Array<juce::File> presets;
    int currentIndex { -1 };

    juce::Label      nameLabel;
    juce::TextButton prevBtn    { "<" };
    juce::TextButton nextBtn    { ">" };
    juce::TextButton saveBtn    { "Save" };
    juce::TextButton saveAsBtn  { "Save As" };
    juce::TextButton deleteBtn  { "Delete" };

    std::unique_ptr<juce::AlertWindow> alertWindow;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetPanel)
};
