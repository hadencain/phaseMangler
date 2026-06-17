cmake_minimum_required(VERSION 3.22)
project(PhaseMangler VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (WIN32)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE INTERNAL "")
endif()

include(FetchContent)
FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        7.0.12
    GIT_SHALLOW    TRUE)
FetchContent_MakeAvailable(JUCE)

option(PHASEMANGLER_ENABLE_TESTS "Build offline tests" OFF)

juce_add_plugin(PhaseMangler
    COMPANY_NAME                "haden"
    PLUGIN_MANUFACTURER_CODE    HCPM
    PLUGIN_CODE                 PM01
    FORMATS                     VST3 Standalone
    IS_SYNTH                    FALSE
    NEEDS_MIDI_INPUT            FALSE
    NEEDS_MIDI_OUTPUT           FALSE
    IS_MIDI_EFFECT              FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD     TRUE
    VST3_CATEGORIES             "Fx"
    PRODUCT_NAME                "PhaseMangler")

target_sources(PhaseMangler PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/UI/LookAndFeel.cpp
    Source/UI/FrequencyDisplay.cpp
    Source/UI/BandColumn.cpp
    Source/UI/PresetPanel/PresetPanel.cpp
    Source/DSP/PhaseScrambler.cpp
    $<$<BOOL:${PHASEMANGLER_ENABLE_TESTS}>:Source/Tests/PhaseScramblerTests.cpp>
)

target_compile_definitions(PhaseMangler PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    CMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}"
    $<$<BOOL:${PHASEMANGLER_ENABLE_TESTS}>:PHASEMANGLER_ENABLE_TESTS=1>)

target_include_directories(PhaseMangler PRIVATE Source)

target_link_libraries(PhaseMangler PRIVATE
    juce::juce_audio_basics
    juce::juce_audio_formats
    juce::juce_audio_plugin_client
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_core
    juce::juce_data_structures
    juce::juce_dsp
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra)
