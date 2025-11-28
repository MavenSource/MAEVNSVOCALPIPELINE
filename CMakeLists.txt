cmake_minimum_required(VERSION 3.20)

project(MAEVN VERSION 1.0.0 LANGUAGES CXX)

# C++17 standard required for modern C++ features
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# JUCE path - can be set via command line: -DJUCE_PATH=/path/to/JUCE
set(JUCE_PATH "" CACHE PATH "Path to JUCE framework")
if(NOT JUCE_PATH)
    message(FATAL_ERROR "JUCE_PATH must be set. Use: -DJUCE_PATH=/path/to/JUCE")
endif()

# ONNX Runtime path - can be set via command line: -DONNXRUNTIME_PATH=/path/to/onnxruntime
set(ONNXRUNTIME_PATH "" CACHE PATH "Path to ONNX Runtime")
if(NOT ONNXRUNTIME_PATH)
    message(FATAL_ERROR "ONNXRUNTIME_PATH must be set. Use: -DONNXRUNTIME_PATH=/path/to/onnxruntime")
endif()

# Add JUCE
add_subdirectory(${JUCE_PATH} JUCE)

# ONNX Runtime include and library paths
include_directories(${ONNXRUNTIME_PATH}/include)
link_directories(${ONNXRUNTIME_PATH}/lib)

# Plugin formats to build
set(PLUGIN_FORMATS VST3 Standalone)
if(APPLE)
    list(APPEND PLUGIN_FORMATS AU)
endif()

# MAEVN VST3 Plugin
juce_add_plugin(MAEVN
    COMPANY_NAME "MavenSource"
    PLUGIN_MANUFACTURER_CODE MvnS
    PLUGIN_CODE Mavn
    FORMATS ${PLUGIN_FORMATS}
    PRODUCT_NAME "MAEVN"
    VST3_CATEGORIES "Fx" "Instrument" "Generator"
    AU_MAIN_TYPE "kAudioUnitType_MusicEffect"
    COPY_PLUGIN_AFTER_BUILD TRUE
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
)

# Source files
target_sources(MAEVN
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
        Source/OnnxEngine.cpp
        Source/OnnxEngine.h
        Source/PatternEngine.cpp
        Source/PatternEngine.h
        Source/AIFXEngine.cpp
        Source/AIFXEngine.h
        Source/CinematicAudioEnhancer.cpp
        Source/CinematicAudioEnhancer.h
        Source/TimelineLane.cpp
        Source/TimelineLane.h
        Source/FXPreset.cpp
        Source/FXPreset.h
        Source/FXPresetManager.cpp
        Source/FXPresetManager.h
        Source/PresetBrowserComponent.cpp
        Source/PresetBrowserComponent.h
        Source/GlobalUndoManager.cpp
        Source/GlobalUndoManager.h
        Source/UndoHistoryComponent.cpp
        Source/UndoHistoryComponent.h
        Source/Utilities.h
        Source/DSPModules.h
        Source/LegendaryProducerFXSuiteUltimate.cpp
        Source/LegendaryProducerFXSuiteUltimate.h
)

# Preprocessor definitions
target_compile_definitions(MAEVN
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_REPORT_APP_USAGE=0
)

# Link libraries
target_link_libraries(MAEVN
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# Link ONNX Runtime
if(WIN32)
    target_link_libraries(MAEVN PRIVATE onnxruntime)
elseif(APPLE)
    target_link_libraries(MAEVN PRIVATE onnxruntime)
else()
    target_link_libraries(MAEVN PRIVATE onnxruntime)
endif()

# Set optimization flags
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(MSVC)
        target_compile_options(MAEVN PRIVATE /O2 /Oi /GL)
    else()
        target_compile_options(MAEVN PRIVATE -O3 -march=native)
    endif()
endif()

# Copy Models and Presets to build directory
add_custom_command(TARGET MAEVN POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/Models
        $<TARGET_FILE_DIR:MAEVN>/Models
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/Presets
        $<TARGET_FILE_DIR:MAEVN>/Presets
    COMMENT "Copying Models and Presets to build directory"
)

message(STATUS "MAEVN VST3 Configuration Complete")
message(STATUS "  - JUCE Path: ${JUCE_PATH}")
message(STATUS "  - ONNX Runtime Path: ${ONNXRUNTIME_PATH}")
message(STATUS "  - Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  - Plugin Formats: ${PLUGIN_FORMATS}")
