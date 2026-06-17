#include "PresetPanel.h"

namespace {
    const juce::String kExt { ".xml" };
}

PresetPanel::PresetPanel(juce::AudioProcessorValueTreeState& a, const juce::String& name)
    : apvts(a), pluginName(name)
{
    nameLabel.setJustificationType(juce::Justification::centred);
    nameLabel.setInterceptsMouseClicks(false, false);
    addAndMakeVisible(nameLabel);

    prevBtn.onClick   = [this] { if (currentIndex > 0) loadPreset(currentIndex - 1); };
    nextBtn.onClick   = [this] { if (currentIndex < (int)presets.size() - 1) loadPreset(currentIndex + 1); };
    saveBtn.onClick   = [this] { saveCurrentPreset(); };
    saveAsBtn.onClick = [this] { savePresetAs(); };
    deleteBtn.onClick = [this] { deleteCurrentPreset(); };

    for (auto* b : { &prevBtn, &nextBtn, &saveBtn, &saveAsBtn, &deleteBtn })
        addAndMakeVisible(b);

    refresh();
}

void PresetPanel::resized()
{
    auto r = getLocalBounds().reduced(4, 4);
    prevBtn  .setBounds(r.removeFromLeft(24));  r.removeFromLeft(2);
    nextBtn  .setBounds(r.removeFromLeft(24));  r.removeFromLeft(6);
    deleteBtn.setBounds(r.removeFromRight(48)); r.removeFromRight(4);
    saveAsBtn.setBounds(r.removeFromRight(56)); r.removeFromRight(4);
    saveBtn  .setBounds(r.removeFromRight(40)); r.removeFromRight(8);
    nameLabel.setBounds(r);
}

juce::File PresetPanel::presetDirectory() const
{
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
               .getChildFile(pluginName)
               .getChildFile("Presets");
}

juce::String PresetPanel::currentName() const
{
    if (currentIndex >= 0 && currentIndex < (int)presets.size())
        return presets[currentIndex].getFileNameWithoutExtension();
    return "Default";
}

void PresetPanel::refresh()
{
    presets.clear();
    auto dir = presetDirectory();
    dir.createDirectory();

    for (auto& f : dir.findChildFiles(juce::File::findFiles, false, "*" + kExt))
        presets.add(f);
    presets.sort();

    if (presets.isEmpty())
        currentIndex = -1;
    else if (currentIndex >= (int)presets.size())
        currentIndex = (int)presets.size() - 1;

    nameLabel.setText(currentName(), juce::dontSendNotification);
}

void PresetPanel::loadPreset(int index)
{
    if (index < 0 || index >= (int)presets.size()) return;
    currentIndex = index;

    if (auto xml = juce::XmlDocument::parse(presets[index]))
    {
        auto state = juce::ValueTree::fromXml(*xml);
        if (state.isValid())
            apvts.replaceState(state);
    }
    nameLabel.setText(currentName(), juce::dontSendNotification);
}

void PresetPanel::saveCurrentPreset()
{
    if (currentIndex < 0) { savePresetAs(); return; }
    if (auto xml = apvts.copyState().createXml())
        xml->writeTo(presets[currentIndex]);
}

void PresetPanel::doSaveAs(const juce::String& name)
{
    auto file = presetDirectory().getChildFile(name + kExt);
    if (auto xml = apvts.copyState().createXml())
        xml->writeTo(file);

    refresh();
    for (int i = 0; i < (int)presets.size(); ++i)
        if (presets[i] == file) { currentIndex = i; break; }
    nameLabel.setText(currentName(), juce::dontSendNotification);
}

void PresetPanel::savePresetAs()
{
    alertWindow = std::make_unique<juce::AlertWindow>(
        "Save Preset", "Enter a name:", juce::MessageBoxIconType::QuestionIcon);
    alertWindow->addTextEditor("name", currentName(), "Preset name:");
    alertWindow->addButton("Save",   1, juce::KeyPress(juce::KeyPress::returnKey));
    alertWindow->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));
    alertWindow->enterModalState(true,
        juce::ModalCallbackFunction::create([this](int result) {
            if (result == 1)
            {
                auto name = alertWindow->getTextEditorContents("name").trim();
                if (name.isNotEmpty()) doSaveAs(name);
            }
            alertWindow.reset();
        }), false);
}

void PresetPanel::deleteCurrentPreset()
{
    if (currentIndex < 0 || currentIndex >= (int)presets.size()) return;
    int prev = currentIndex;
    presets[currentIndex].deleteFile();
    refresh();
    if (!presets.isEmpty())
        currentIndex = juce::jlimit(0, (int)presets.size() - 1, prev);
    nameLabel.setText(currentName(), juce::dontSendNotification);
}
