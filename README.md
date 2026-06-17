# PhaseMangler

A VST3 spectral destruction effect. Input audio is transformed via overlap-add FFT and the phase spectrum is stochastically scrambled across three independent frequency bands. Magnitude is untouched — the spectral shape survives, the waveform is destroyed.

At wet mix 0 the plugin is transparent. Raise it and the signal begins to dissolve.

---

## How it works

```
Input ──────────────────────────────────────────→ Dry out
  │
  └→ [PhaseScrambler (2048pt FFT, 75% overlap)]
           │
           └─ Per-bin: compound p from band coverage → re-roll phaseOffset[k]
           └─ Apply: bin[k] = |bin[k]| × exp(i × (arg + phaseOffset[k]))
           │
           └→ Wet out
```

Three independent floating bands each define a frequency range, scramble rate (probability per FFT frame that a covered bin re-rolls its offset), and depth (max offset range ±depth×π). Overlapping bands compound: p = 1 − ∏(1 − rate_n). Phase offsets persist between frames — bins hold their value until they re-roll, producing flutter/static texture rather than white noise.

---

## Parameters

| Band | Parameter | Range | Default |
|------|-----------|-------|---------|
| 1    | Enable    | bool  | true    |
| 1    | Center    | 20–20kHz | 200 Hz |
| 1    | Width     | 0.1–4 oct | 1.0 oct |
| 1    | Rate      | 0–1   | 0.3     |
| 1    | Depth     | 0–1   | 0.5     |
| 2    | Enable    | bool  | false   |
| 2    | Center    | 20–20kHz | 1000 Hz |
| 2–3  | Width/Rate/Depth | same ranges | same defaults |
| 3    | Center    | 20–20kHz | 5000 Hz |
| —    | Wet Mix   | 0–1   | 0.0     |

Latency: 2048 samples (FFT window size). Reported to host via `getLatencySamples()`.

---

## Build

**Requirements:** CMake 3.22+, Ninja, MSVC (Visual Studio 2022+ Build Tools), Windows.

JUCE 7.0.12 is fetched automatically via CMake FetchContent.

```bash
# From Developer Command Prompt (vcvars64.bat active)
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

**Standalone:**
```
build/PhaseMangler_artefacts/Release/Standalone/PhaseMangler.exe
```

**VST3** (admin copy required):
```
build/PhaseMangler_artefacts/Release/VST3/PhaseMangler.vst3
```
Copy to `C:\Program Files\Common Files\VST3\`.

---

## Stack

- JUCE 7.0.12
- C++17
- CMake / Ninja / MSVC
- VST3 + Standalone targets
- Windows-first
