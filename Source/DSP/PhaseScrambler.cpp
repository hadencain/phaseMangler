#include "PhaseScrambler.h"
#include <cmath>

PhaseScrambler::PhaseScrambler()
    : fft(FFT_ORDER),
      window((size_t)FFT_SIZE, juce::dsp::WindowingFunction<float>::hann, true)
{}

void PhaseScrambler::prepare(double sr, int /*maxBlockSize*/)
{
    sampleRate = sr;
    reset();
}

void PhaseScrambler::reset()
{
    inputHistory.fill(0.0f);
    inputHistoryPos = 0;
    hopSampleCount  = 0;
    fftWorkBuf.fill(0.0f);
    phaseOffset.fill(0.0f);
    outputOverlapBuf.fill(0.0f);
    outputReadPos = 0;
    rng.setSeed(12345);
}

void PhaseScrambler::processFrame(const BandDescriptor* bands, int numBands)
{
    // 1. Assemble analysis frame from circular input history
    for (int i = 0; i < FFT_SIZE; ++i)
    {
        int src = (inputHistoryPos + i) % FFT_SIZE;
        fftWorkBuf[i * 2]     = inputHistory[src];
        fftWorkBuf[i * 2 + 1] = 0.0f;
    }

    // 2. Apply Hann window to real part
    {
        float tempReal[FFT_SIZE];
        for (int i = 0; i < FFT_SIZE; ++i) tempReal[i] = fftWorkBuf[i * 2];
        window.multiplyWithWindowingTable(tempReal, (size_t)FFT_SIZE);
        for (int i = 0; i < FFT_SIZE; ++i) fftWorkBuf[i * 2] = tempReal[i];
    }

    // 3. Forward FFT
    fft.performRealOnlyForwardTransform(fftWorkBuf.data(), true);

    // 4. Per-bin stochastic phase scramble
    for (int k = 0; k < NUM_BINS; ++k)
    {
        float compoundKeep = 1.0f;
        float maxDepth = 0.0f;
        for (int b = 0; b < numBands; ++b)
        {
            if (!bands[b].enabled) continue;
            auto [lo, hi] = bands[b].binRange(FFT_SIZE, sampleRate);
            if (lo > hi || k < lo || k > hi) continue;
            compoundKeep *= (1.0f - bands[b].rate);
            maxDepth = juce::jmax(maxDepth, bands[b].depth);
        }
        float p = 1.0f - compoundKeep;

        if (p > 0.0f && rng.nextFloat() < p)
        {
            float range = maxDepth * juce::MathConstants<float>::pi;
            phaseOffset[k] = (rng.nextFloat() * 2.0f - 1.0f) * range;
        }

        float re = fftWorkBuf[k * 2];
        float im = fftWorkBuf[k * 2 + 1];
        float mag = std::sqrt(re * re + im * im);
        float arg = std::atan2(im, re);
        float newArg = arg + phaseOffset[k];
        fftWorkBuf[k * 2]     = mag * std::cos(newArg);
        fftWorkBuf[k * 2 + 1] = mag * std::sin(newArg);
    }

    // 5. Inverse FFT
    fft.performRealOnlyInverseTransform(fftWorkBuf.data());

    // 6. Synthesis window + overlap-add
    {
        float tempReal[FFT_SIZE];
        for (int i = 0; i < FFT_SIZE; ++i) tempReal[i] = fftWorkBuf[i * 2];
        window.multiplyWithWindowingTable(tempReal, (size_t)FFT_SIZE);
        for (int i = 0; i < FFT_SIZE; ++i)
            outputOverlapBuf[(outputReadPos + i) % FFT_SIZE] += tempReal[i];
    }
}

void PhaseScrambler::processBlock(juce::AudioBuffer<float>& buffer,
                                   const BandDescriptor* bands, int numBands,
                                   int numSamples, float wetMix)
{
    const int numChannels = buffer.getNumChannels();
    const bool hasStereo  = numChannels >= 2;

    for (int i = 0; i < numSamples; ++i)
    {
        float dry0 = buffer.getSample(0, i);
        float dry1 = hasStereo ? buffer.getSample(1, i) : dry0;
        float mono = hasStereo ? (dry0 + dry1) * 0.5f : dry0;

        inputHistory[inputHistoryPos] = mono;
        inputHistoryPos = (inputHistoryPos + 1) % FFT_SIZE;

        ++hopSampleCount;
        if (hopSampleCount >= HOP_SIZE)
        {
            hopSampleCount = 0;
            processFrame(bands, numBands);
        }

        float wet = outputOverlapBuf[outputReadPos];
        outputOverlapBuf[outputReadPos] = 0.0f;
        outputReadPos = (outputReadPos + 1) % FFT_SIZE;

        buffer.setSample(0, i, dry0 * (1.0f - wetMix) + wet * wetMix);
        if (hasStereo)
            buffer.setSample(1, i, dry1 * (1.0f - wetMix) + wet * wetMix);
    }
}
