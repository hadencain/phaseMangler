# PhaseMangler

VST3 spectral phase scrambler. JUCE 7.0.12, CMake, C++17, Windows.

## Build

```
# vcvars64.bat must be active
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

VS Build Tools path: `C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat`

Standalone: `build/PhaseMangler_artefacts/Release/Standalone/PhaseMangler.exe`

## Architecture

- `Source/DSP/BandDescriptor.h` — plain struct, `binRange()` method
- `Source/DSP/PhaseScrambler.h/.cpp` — 2048pt FFT, 75% overlap, persistent `phaseOffset[1025]` array
- `Source/PluginProcessor.h/.cpp` — 16 APVTS params, constructs BandDescriptor[3] per block
- `Source/PluginEditor.h/.cpp` — 600×360, PresetPanel + FrequencyDisplay + 3×BandColumn
- `Source/UI/FrequencyDisplay` — log-scale 20Hz–20kHz, colored translucent band rects
- `Source/UI/BandColumn` — enable toggle + Center/Width/Rate/Depth knobs per band

## Known issue fixed during scaffold

`juce::dsp::FFT` has no default constructor — MSVC cannot synthesize `PhaseScrambler()`
from in-class member initializers alone. Explicit constructor defined in `PhaseScrambler.cpp`
with initializer list: `fft(FFT_ORDER), window(FFT_SIZE, hann, true)`.
