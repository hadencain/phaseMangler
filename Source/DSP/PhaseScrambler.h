#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <array>
#include <vector>
#include "BandDescriptor.h"

class PhaseScrambler
{
public:
    static constexpr int FFT_ORDER = 11;
    static constexpr int FFT_SIZE  = 1 << FFT_ORDER;  // 2048
    static constexpr int HOP_SIZE  = FFT_SIZE / 4;    // 512 (75% overlap)
    static constexpr int NUM_BINS  = FFT_SIZE / 2 + 1; // 1025

    PhaseScrambler();

    void prepare(double sampleRate, int maxBlockSize);
    void processBlock(juce::AudioBuffer<float>& buffer,
                      const BandDescriptor* bands, int numBands,
                      int numSamples, float wetMix);
    void reset();

    int getLatencySamples() const noexcept { return FFT_SIZE; }

private:
    double sampleRate = 44100.0;

    juce::dsp::FFT fft;
    juce::dsp::WindowingFunction<float> window;

    // Input side
    std::array<float, FFT_SIZE> inputHistory {};
    int inputHistoryPos = 0;
    int hopSampleCount  = 0;

    // FFT working buffer (interleaved re/im, size 2×FFT_SIZE)
    std::array<float, FFT_SIZE * 2> fftWorkBuf {};

    // Persistent phase offset per bin — holds value until re-rolled
    std::array<float, NUM_BINS> phaseOffset {};

    // Output overlap-add buffer (ring, size FFT_SIZE)
    std::array<float, FFT_SIZE> outputOverlapBuf {};
    int outputReadPos = 0;

    juce::Random rng;

    void processFrame(const BandDescriptor* bands, int numBands);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PhaseScrambler)
};
