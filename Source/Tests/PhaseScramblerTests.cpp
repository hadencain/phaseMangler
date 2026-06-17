#ifdef PHASEMANGLER_ENABLE_TESTS

#include "../DSP/PhaseScrambler.h"
#include <juce_core/juce_core.h>
#include <cmath>

static float rmsOf(const juce::AudioBuffer<float>& b)
{
    if (b.getNumSamples() == 0) return 0.0f;
    double sum = 0.0;
    const float* r = b.getReadPointer(0);
    for (int i = 0; i < b.getNumSamples(); ++i) sum += double(r[i]) * r[i];
    return float(std::sqrt(sum / b.getNumSamples()));
}

static float toDb(float rms) { return 20.0f * std::log10(rms + 1e-30f); }

struct Test { const char* name; bool(*fn)(); };

static bool testBypass()
{
    PhaseScrambler s;
    s.prepare(44100.0, 512);
    BandDescriptor b1; b1.enabled = true; b1.centerHz = 1000; b1.widthOctaves = 3;
    b1.rate = 1.0f; b1.depth = 1.0f;
    juce::AudioBuffer<float> buf(2, 512);
    buf.setSample(0, 0, 0.7f); buf.setSample(1, 0, 0.7f);
    s.processBlock(buf, &b1, 1, 512, 0.0f);
    return std::abs(buf.getSample(0, 0) - 0.7f) < 1e-6f;
}

static bool testSilentInput()
{
    PhaseScrambler s;
    s.prepare(44100.0, 512);
    BandDescriptor bands[3];
    for (auto& b : bands) { b.enabled=true; b.centerHz=1000; b.widthOctaves=4; b.rate=1; b.depth=1; }
    juce::AudioBuffer<float> buf(2, 512);
    for (int i = 0; i < 264; ++i)
    {
        buf.clear();
        s.processBlock(buf, bands, 3, 512, 1.0f);
    }
    return toDb(rmsOf(buf)) < -120.0f;
}

static bool testMagnitudePreservation()
{
    PhaseScrambler s;
    s.prepare(44100.0, 512);
    BandDescriptor b1; b1.enabled = true; b1.centerHz = 1000; b1.widthOctaves = 10;
    b1.rate = 1.0f; b1.depth = 1.0f;
    juce::AudioBuffer<float> inBuf(1, 512), outBuf(1, 512);
    float inputRmsSum = 0.0f, outputRmsSum = 0.0f;
    int nBlocks = 86;
    for (int i = 0; i < nBlocks; ++i)
    {
        float phase = float(i) * 512.0f / 44100.0f * 440.0f * 2.0f * 3.14159f;
        for (int j = 0; j < 512; ++j)
            inBuf.setSample(0, j, std::sin(phase + float(j) / 44100.0f * 440.0f * 2.0f * 3.14159f));
        inputRmsSum += rmsOf(inBuf);
        outBuf.copyFrom(0, 0, inBuf, 0, 0, 512);
        s.processBlock(outBuf, &b1, 1, 512, 1.0f);
        outputRmsSum += rmsOf(outBuf);
    }
    float inDb  = toDb(inputRmsSum  / nBlocks);
    float outDb = toDb(outputRmsSum / nBlocks);
    return std::abs(inDb - outDb) < 1.0f;
}

static bool testRateZeroStability()
{
    PhaseScrambler s;
    s.prepare(44100.0, 512);
    BandDescriptor b1; b1.enabled = true; b1.centerHz = 1000; b1.widthOctaves = 3;
    b1.rate = 0.0f; b1.depth = 1.0f;
    juce::AudioBuffer<float> buf(1, 512);
    juce::AudioBuffer<float> ref(1, 512);
    for (int blk = 0; blk < 5; ++blk)
    {
        buf.clear(); for (int i=0;i<512;++i) buf.setSample(0,i,0.1f);
        s.processBlock(buf, &b1, 1, 512, 1.0f);
    }
    ref.copyFrom(0, 0, buf, 0, 0, 512);
    for (int blk = 0; blk < 100; ++blk)
    {
        buf.clear(); for (int i=0;i<512;++i) buf.setSample(0,i,0.1f);
        juce::AudioBuffer<float> tmp(1,512); tmp.copyFrom(0,0,buf,0,0,512);
        s.processBlock(tmp, &b1, 1, 512, 1.0f);
        for (int i = 0; i < 512; ++i)
            if (std::abs(tmp.getSample(0,i) - ref.getSample(0,i)) > 1e-5f) return false;
    }
    return true;
}

static bool testNarrowBandIsolation()
{
    BandDescriptor narrow; narrow.enabled = true; narrow.centerHz = 1.0f; narrow.widthOctaves = 0.1f;
    auto [lo, hi] = narrow.binRange(2048, 44100.0);
    return lo >= 0 && hi <= 2048/2;
}

static bool testCompoundProbability()
{
    PhaseScrambler s;
    s.prepare(44100.0, 512);
    BandDescriptor b[2];
    b[0].enabled = true; b[0].centerHz = 1000; b[0].widthOctaves = 1; b[0].rate = 0.5f; b[0].depth = 1.0f;
    b[1].enabled = true; b[1].centerHz = 1000; b[1].widthOctaves = 1; b[1].rate = 0.5f; b[1].depth = 1.0f;
    int blocks = 10000;
    juce::AudioBuffer<float> buf(1, 1);
    int nonZeroFrames = 0;
    for (int i = 0; i < blocks; ++i)
    {
        buf.setSample(0, 0, 0.1f);
        s.processBlock(buf, b, 2, 1, 1.0f);
        if (std::abs(buf.getSample(0,0)) > 1e-7f) ++nonZeroFrames;
    }
    return nonZeroFrames > 1000;
}

static bool testPrepareToPlayReinit()
{
    PhaseScrambler s;
    s.prepare(44100.0, 512);
    BandDescriptor b1; b1.enabled = true; b1.centerHz = 1000; b1.widthOctaves = 3;
    b1.rate = 1.0f; b1.depth = 1.0f;
    juce::AudioBuffer<float> buf(2, 512);
    for (int i = 0; i < 512; ++i) { buf.setSample(0,i,1.0f); buf.setSample(1,i,1.0f); }
    for (int blk = 0; blk < 10; ++blk)
        s.processBlock(buf, &b1, 1, 512, 1.0f);
    s.prepare(48000.0, 512);
    buf.clear();
    s.processBlock(buf, &b1, 1, 512, 1.0f);
    return toDb(rmsOf(buf)) < -100.0f;
}

static Test tests[] = {
    { "bypass",                   testBypass               },
    { "silent_input",             testSilentInput          },
    { "magnitude_preservation",   testMagnitudePreservation },
    { "rate_zero_stability",      testRateZeroStability    },
    { "narrow_band_isolation",    testNarrowBandIsolation  },
    { "compound_probability",     testCompoundProbability  },
    { "prepare_reinit",           testPrepareToPlayReinit  },
};

struct PhaseScramblerTestRunner
{
    PhaseScramblerTestRunner()
    {
        int passed = 0, total = int(sizeof(tests) / sizeof(tests[0]));
        for (auto& t : tests)
        {
            bool ok = t.fn();
            juce::Logger::writeToLog(juce::String(ok ? "[PASS] " : "[FAIL] ") + t.name);
            if (ok) ++passed;
        }
        juce::Logger::writeToLog("PhaseScrambler tests: " + juce::String(passed) + "/" + juce::String(total));
    }
};

static PhaseScramblerTestRunner runnerInstance;

#endif // PHASEMANGLER_ENABLE_TESTS
