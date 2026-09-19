cmake_minimum_required(VERSION 3.22)

project(AstrophiluxNebula
    VERSION 0.6.0
    LANGUAGES C CXX
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.8
)

FetchContent_MakeAvailable(JUCE)

juce_add_plugin(AstrophiluxNebula
    COMPANY_NAME "Astrophilux"
    BUNDLE_ID "com.astrophilux.nebula"
    PLUGIN_MANUFACTURER_CODE Astr
    PLUGIN_CODE Neb1
    FORMATS VST3
    PRODUCT_NAME "Astrophilux Nebula"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
)

juce_generate_juce_header(AstrophiluxNebula)

juce_add_binary_data(NebulaAssets
    NAMESPACE BinaryData
    SOURCES
        Assets/Nebula_Source_C3.wav
        Assets/Infinity.png
)

target_sources(AstrophiluxNebula PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
)

target_compile_definitions(AstrophiluxNebula PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(AstrophiluxNebula PRIVATE
    NebulaAssets
    juce::juce_audio_utils
    juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags
)
