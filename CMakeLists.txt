cmake_minimum_required(VERSION 3.22)

project(ASCII_DSP VERSION 0.1.0)

set(JUCE_ENABLE_MODULE_SOURCE_GROUPS ON)

add_definitions(
    -DJUCE_VST3_CAN_REPLACE_VST2=0
    -DJUCE_PLUGINHOST_VST=0
)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(JUCE)

juce_add_plugin(ASCII_DSP
    COMPANY_NAME "ASCII_DSP"
    BUNDLE_ID com.asciidsp.plugin
    PLUGIN_MANUFACTURER_CODE AsDp
    PLUGIN_CODE Asc1

    FORMATS VST3 Standalone

    PRODUCT_NAME "ASCII DSP"

    VST3_CAN_REPLACE_VST2 FALSE
)

target_sources(ASCII_DSP
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
)

target_compile_definitions(ASCII_DSP
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
)

target_link_libraries(ASCII_DSP
    PRIVATE
        juce::juce_audio_utils
)