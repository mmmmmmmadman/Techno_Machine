cmake_minimum_required(VERSION 3.22)

project(TechnoMachine VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(JUCE)

juce_add_gui_app(TechnoMachine
    PRODUCT_NAME "Techno Machine"
    COMPANY_NAME "MADZINE"
    BUNDLE_ID "com.madzine.technomachine"
)

juce_generate_juce_header(TechnoMachine)

# Embed fonts as binary data - Inter Thin for cross-platform
juce_add_binary_data(FontData
    SOURCES
        Resources/Fonts/Inter-Thin.ttf
)

target_sources(TechnoMachine
    PRIVATE
        Source/Main.cpp
        Source/MainComponent.cpp
        Source/Core/AudioEngine.cpp
        Source/Core/Clock.cpp
        Source/Core/Transport.cpp
        Source/Core/CVOutputRouter.cpp
        Source/UI/AudioSettingsWindow.cpp
)

target_compile_definitions(TechnoMachine
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:TechnoMachine,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:TechnoMachine,JUCE_VERSION>"
)

target_link_libraries(TechnoMachine
    PRIVATE
        FontData
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_data_structures
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags
)
