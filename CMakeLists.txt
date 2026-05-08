cmake_minimum_required(VERSION 3.22)
project(AnttiImageSynth VERSION 1.0.0)

add_subdirectory(JUCE)

juce_add_plugin(AnttiImageSynth
    PRODUCT_NAME "Antti Image Synth"
    VERSION 1.0.0
    PLUGIN_MANUFACTURER_CODE "ALab"
    PLUGIN_CODE "ImgS"
    FORMATS VST3 Standalone
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE)

target_sources(AnttiImageSynth PRIVATE
    PluginProcessor.cpp
    PluginEditor.cpp)

target_link_libraries(AnttiImageSynth PRIVATE
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_gui_extra
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags)

target_compile_definitions(AnttiImageSynth PRIVATE
    JUCE_VST3_CAN_REPLACE_VST2=0)

juce_generate_juce_header(AnttiImageSynth)