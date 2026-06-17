---
project: PhaseMangler
date: 2026-06-17
status: awaiting-review
---

### What this is

A VST3 spectral destruction effect. Input audio is transformed via overlap-add FFT; each block, a stochastic phase scrambler applies random phase offsets to frequency bins grouped into three independent floating bands. Magnitude spectrum is untouched — the spectral shape of the source survives, but the waveform is destroyed. At low rate the texture holds a consistent scrambled state and occasionally stutters. At high rate it dissolves into flutter and static. Overlapping bands compound their scramble probability. Dry signal is unaffected at wet mix 0. Stack: JUCE 7.0.12, CMake, VST3 + Standalone, Windows-first.

---

### Success criteria

1. Wet mix at 0: output is bit-identical to input regardless of band configuration.
2. Silent input with all bands active at rate=1, depth=1: output RMS < -120dBFS. No idle noise or clicks.
3. Rate=0 on all bands: phase offset array never updates between blocks. Output is a consistent (non-random) transformation of the input, not new noise each block.
4. A single band covering all bins at rate=1, depth=1: output RMS within 1dB of input RMS. Phase scrambling must not alter magnitude.
5. A band so narrow it covers 0 FFT bins has no audible effect. No crash, no parameter snap.
6. Two bands with rate=0.5 covering the same bin produce an effective scramble rate of ~0.75 (verified by bin flip count over many blocks).
7. prepareToPlay at a new sample rate clears the phase offset array. The first output block after reinit starts from a clean state.
8. Standalone launches, accepts system audio input, passes dry signal with wet mix at 0 before any interaction.

---

### Stack

- JUCE 7.0.12 via FetchContent (pinned)
- CMake 3.22+, Ninja, MSVC static runtime
- C++17
- VST3 + Standalone targets
- No grain primitives borrowed from granulator — FFT engine written from scratch using `juce::dsp::FFT`

---

### Architecture — three components, one direction of knowledge

```
Input ──────────────────────────────────────────→ Dry out
  │
  └→ [PhaseScrambler] ← BandDescriptor[3] from PluginProcessor
          │
          └→ Wet out
```

**PhaseScrambler** owns the FFT engine (overlap-add), the persistent per-bin phase offset array, and the per-block stochastic update. It has no knowledge of the processor or APVTS. It receives 3 `BandDescriptor` structs by value each block — no pointers back into the processor.

**BandDescriptor** is a plain struct: `centerHz`, `widthOctaves`, `rate`, `depth`, `enabled`. Provides a method to compute the bin range `[binLo, binHi]` given FFT size and sample rate.

**PluginProcessor** owns APVTS (16 parameters), reads parameter values each block, constructs 3 BandDescriptors, passes them to PhaseScrambler. SmoothedValue on wetMix.

**PluginEditor** owns frequency display + 3 knob columns. No reads from PhaseScrambler state — display is driven by APVTS parameter values only.

---

### Components

**PhaseScrambler**

FFT: 2048 points, Hann window, 75% overlap (hop size = 512 samples). Latency = 2048 samples — reported via `getLatencySamples()`. Implemented with `juce::dsp::FFT`.

Per-block logic for bin k (frequency `f = k × sampleRate / FFT_size`):
1. For each active BandDescriptor: test if `f ∈ [center / 2^(width/2), center × 2^(width/2)]`
2. Compound probability across covering bands: `p = 1 − ∏(1 − rate_n)`
3. Roll: if `rand() < p`, replace `phaseOffset[k]` with uniform random in `[−depth×π, +depth×π]`
4. Apply: `bin[k] = |bin[k]| × exp(i × (arg(bin[k]) + phaseOffset[k]))`

The phase offset array is persistent across blocks. Bins hold their offset until they re-roll. This is what produces the flutter/static texture rather than per-block white noise.

Prepare: allocate input/output overlap buffers, FFT working buffer (size 2×FFT_size), phase offset array (size FFT_size/2 + 1), all zeroed. Re-allocate on sample rate or block size change.

Reset: zero phase offset array, zero overlap buffers.

**BandDescriptor**

```cpp
struct BandDescriptor {
    float centerHz      = 1000.0f;
    float widthOctaves  = 1.0f;
    float rate          = 0.3f;  // [0, 1] probability per block per bin
    float depth         = 0.5f;  // [0, 1] mapped to [0, π] max offset
    bool  enabled       = false;

    std::pair<int,int> binRange(int fftSize, double sampleRate) const noexcept;
};
```

If `binRange` returns `{k, k}` or `binLo > binHi`, the band covers 0 bins — has no effect.

**PluginProcessor**

APVTS owns all 16 parameters. Each block: reads raw parameter values → constructs `BandDescriptor[3]` → calls `scrambler.processBlock(buffer, bands, numSamples, wetMix)`.

`SmoothedValue<float>` on `wetMix` (5ms ramp) prevents clicks during automation.

State serialization: APVTS XML via `getStateInformation` / `setStateInformation`.

**PluginEditor**

600×360. Top 30px: PresetPanel. Next 120px: FrequencyDisplay. Remaining height: 3-column knob grid. Bottom 14px: build stamp.

FrequencyDisplay: log-scale 20Hz–20kHz axis. Each active band drawn as a translucent colored rectangle. Band 1 = blue, Band 2 = amber, Band 3 = red. Driven by APVTS listener — repaints when any band parameter changes. Non-interactive (no drag handles).

3 knob columns: each has a colored header matching the display, an enable TextButton, then 4 rotary sliders (Center, Width, Rate, Depth) with TextBoxBelow labels. Attachment via `SliderAttachment` and `ButtonAttachment`.

---

### Parameter surface

| ID | Name | Range | Default |
|----|------|-------|---------|
| `band1_enable` | Enable | bool | true |
| `band1_center` | Center | 20–20k Hz, log | 200 Hz |
| `band1_width` | Width | 0.1–4 oct | 1.0 oct |
| `band1_rate` | Rate | 0–1 | 0.3 |
| `band1_depth` | Depth | 0–1 | 0.5 |
| `band2_enable` | Enable | bool | false |
| `band2_center` | Center | 20–20k Hz, log | 1000 Hz |
| `band2_width` | Width | 0.1–4 oct | 1.0 oct |
| `band2_rate` | Rate | 0–1 | 0.3 |
| `band2_depth` | Depth | 0–1 | 0.5 |
| `band3_enable` | Enable | bool | false |
| `band3_center` | Center | 20–20k Hz, log | 5000 Hz |
| `band3_width` | Width | 0.1–4 oct | 1.0 oct |
| `band3_rate` | Rate | 0–1 | 0.3 |
| `band3_depth` | Depth | 0–1 | 0.5 |
| `wetMix` | Wet Mix | 0–1 | 0.0 |

Only band 1 enabled by default. At defaults: a single mid-low band (200Hz, 1 oct wide) scrambling at moderate rate and depth. Wet mix 0 = transparent until opted in.

---

### Error handling

- **Band covers 0 bins** (width too narrow, or center pushed to edge of spectrum): `binRange` returns degenerate range, scrambler skips it silently.
- **All bands disabled**: PhaseScrambler loop still runs but compounds to p=0 everywhere — no phase updates, output is a consistent passthrough of accumulated offsets (which are all 0 if never updated).
- **prepareToPlay mid-session**: phase array and overlap buffers cleared. Tail from before reinit is discarded. Next block starts clean.
- **Wet mix at 0**: dry path passes buffer through unmodified before PhaseScrambler is called — scrambler still runs (to keep state warm) but its output is scaled to 0.
- **Overlapping bands at rate=1**: compound probability = 1 — bin re-rolls every block. Correct behavior.

---

### Testing

All tests offline (no audio device required). Compiled into Standalone with `-DPHASEMANGLER_ENABLE_TESTS=1`.

1. **Bypass** — wet mix=0, band 1 active at rate=1 depth=1: output buffer == input buffer sample-for-sample.
2. **Silent input** — all bands active at rate=1, depth=1, wet=1: output RMS < -120dBFS after 3s of zeros.
3. **Magnitude preservation** — single band covering full spectrum, rate=1, depth=1, wet=1: output RMS within 1dB of input RMS over a 1s sine tone.
4. **Rate=0 stability** — rate=0 all bands: call processBlock 100×, assert no element of the phase offset array changes value across any block (zero updates total).
5. **Narrow band isolation** — band 1 covers 200–400Hz only (center=283Hz, width=1oct), bands 2+3 disabled: bins outside that range have phaseOffset==0 after 100 blocks at rate=1.
6. **Compound probability** — two bands with rate=0.5 covering the same bin: over 10000 blocks, flip count should be in [7000, 8000] (expected 7500, ±3σ).
7. **prepareToPlay reinit** — prepare at 44100, run 10 blocks with impulse, prepare at 48000, run 1 block of silence: no crash, output RMS < 1e-10.
