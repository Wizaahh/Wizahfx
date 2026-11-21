cmake_minimum_required(VERSION 3.22)
project(WizahFX VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.4
    GIT_SHALLOW ON)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(WizahFX
    COMPANY_NAME "Wizah"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    PLUGIN_MANUFACTURER_CODE Wzah
    PLUGIN_CODE Wfx1
    FORMATS VST3
    PRODUCT_NAME "Wizah FX"
    COPY_PLUGIN_AFTER_BUILD FALSE)

target_sources(WizahFX PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h)

target_compile_definitions(WizahFX PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(WizahFX
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
