#pragma once
#include <cmath>
#include <utility>
#include <juce_core/juce_core.h>

struct BandDescriptor
{
    float centerHz     = 1000.0f;
    float widthOctaves = 1.0f;
    float rate         = 0.3f;   // [0,1] probability per block per bin
    float depth        = 0.5f;   // [0,1] → [0, π] max offset
    bool  enabled      = false;

    // Returns [binLo, binHi] inclusive. If binLo >= binHi the band covers 0 bins.
    std::pair<int,int> binRange(int fftSize, double sampleRate) const noexcept
    {
        if (!enabled || centerHz <= 0.0f || widthOctaves <= 0.0f)
            return {0, -1};
        float halfWidthFactor = std::pow(2.0f, widthOctaves * 0.5f);
        float loHz = centerHz / halfWidthFactor;
        float hiHz = centerHz * halfWidthFactor;
        int lobin = int(loHz * float(fftSize) / float(sampleRate));
        int hibin = int(hiHz * float(fftSize) / float(sampleRate));
        int maxBin = fftSize / 2;
        lobin = juce::jlimit(0, maxBin, lobin);
        hibin = juce::jlimit(0, maxBin, hibin);
        return {lobin, hibin};
    }
};
