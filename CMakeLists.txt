cmake_minimum_required(VERSION 3.22)
project(VocalAIStudio VERSION 0.1.0 LANGUAGES CXX)

option(VAS_FETCH_JUCE "Descargar JUCE automáticamente si no se encuentra localmente" ON)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/JUCE/CMakeLists.txt")
  add_subdirectory(JUCE)
elseif(VAS_FETCH_JUCE)
  include(FetchContent)
  FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.10 GIT_SHALLOW TRUE)
  FetchContent_MakeAvailable(JUCE)
else()
  message(FATAL_ERROR "No se encontró JUCE. Coloca JUCE en ./JUCE o configura VAS_FETCH_JUCE=ON.")
endif()

if(APPLE)
  set(VAS_PLUGIN_FORMATS VST3 AU)
else()
  set(VAS_PLUGIN_FORMATS VST3)
endif()

juce_add_plugin(VocalAIStudio
  COMPANY_NAME "Vocal AI Studio"
  BUNDLE_ID "studio.vocalai.vocal-ai-studio"
  PLUGIN_MANUFACTURER_CODE VoAI
  PLUGIN_CODE VcAI
  FORMATS ${VAS_PLUGIN_FORMATS}
  PRODUCT_NAME "Vocal AI Studio"
  DESCRIPTION "Procesador vocal minimalista"
  IS_SYNTH FALSE
  NEEDS_MIDI_INPUT FALSE
  NEEDS_MIDI_OUTPUT FALSE
  IS_MIDI_EFFECT FALSE
  EDITOR_WANTS_KEYBOARD_FOCUS TRUE
  COPY_PLUGIN_AFTER_BUILD FALSE
)

target_sources(VocalAIStudio PRIVATE
  Source/PluginProcessor.cpp
  Source/PluginProcessor.h
  Source/PluginEditor.cpp
  Source/PluginEditor.h
)
target_compile_features(VocalAIStudio PRIVATE cxx_std_17)
target_compile_definitions(VocalAIStudio PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(VocalAIStudio PRIVATE juce::juce_audio_utils juce::juce_dsp juce::juce_audio_processors PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_lto_flags juce::juce_recommended_warning_flags)

include(CTest)
find_package(Python3 COMPONENTS Interpreter)
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
  add_test(NAME source_contracts COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/Tests/check_source.py ${CMAKE_CURRENT_SOURCE_DIR})
endif()
