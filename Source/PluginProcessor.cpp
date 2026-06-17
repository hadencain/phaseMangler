#include "PluginProcessor.h"
#include "PluginEditor.h"

PhaseManglerProcessor::PhaseManglerProcessor()
    : AudioProcessor(BusesProperties()
          .withInput("Input",   juce::AudioChannelSet::stereo(), true)
          .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "PhaseMangler", createParameterLayout())
{
    setLatencySamples(scrambler.getLatencySamples());
}

juce::AudioProcessorValueTreeState::ParameterLayout
PhaseManglerProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    struct BandDef { int idx; float defaultCenter; bool defaultEnabled; };
    const BandDef defs[3] = { {1, 200.0f, true}, {2, 1000.0f, false}, {3, 5000.0f, false} };

    for (auto& d : defs)
    {
        juce::String pre = "band" + juce::String(d.idx) + "_";
        layout.add(std::make_unique<juce::AudioParameterBool>(
            pre + "enable", "Band " + juce::String(d.idx) + " Enable", d.defaultEnabled));
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            pre + "center", "Band " + juce::String(d.idx) + " Center",
            juce::NormalisableRange<float>(20.0f, 20000.0f, 0.0f, 0.25f),
            d.defaultCenter));
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            pre + "width", "Band " + juce::String(d.idx) + " Width",
            juce::NormalisableRange<float>(0.1f, 4.0f), 1.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            pre + "rate", "Band " + juce::String(d.idx) + " Rate",
            juce::NormalisableRange<float>(0.0f, 1.0f), 0.3f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            pre + "depth", "Band " + juce::String(d.idx) + " Depth",
            juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));
    }

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "wetMix", "Wet Mix",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));

    return layout;
}

void PhaseManglerProcessor::prepareToPlay(double sampleRate, int maxBlockSize)
{
    scrambler.prepare(sampleRate, maxBlockSize);
    smoothWet.reset(sampleRate, 0.005);
    smoothWet.setCurrentAndTargetValue(0.0f);

    pWetMix = apvts.getRawParameterValue("wetMix");
    for (int i = 0; i < 3; ++i)
    {
        juce::String pre = "band" + juce::String(i + 1) + "_";
        bandParams[i].enable = apvts.getRawParameterValue(pre + "enable");
        bandParams[i].center = apvts.getRawParameterValue(pre + "center");
        bandParams[i].width  = apvts.getRawParameterValue(pre + "width");
        bandParams[i].rate   = apvts.getRawParameterValue(pre + "rate");
        bandParams[i].depth  = apvts.getRawParameterValue(pre + "depth");
    }
}

void PhaseManglerProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                          juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    smoothWet.setTargetValue(pWetMix->load());
    float wetMix = smoothWet.getNextValue();

    BandDescriptor bands[3];
    for (int i = 0; i < 3; ++i)
    {
        bands[i].enabled      = bandParams[i].enable->load() > 0.5f;
        bands[i].centerHz     = bandParams[i].center->load();
        bands[i].widthOctaves = bandParams[i].width->load();
        bands[i].rate         = bandParams[i].rate->load();
        bands[i].depth        = bandParams[i].depth->load();
    }

    scrambler.processBlock(buffer, bands, 3, buffer.getNumSamples(), wetMix);
}

void PhaseManglerProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void PhaseManglerProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml && xml->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessorEditor* PhaseManglerProcessor::createEditor()
{
    return new PhaseManglerEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PhaseManglerProcessor();
}
