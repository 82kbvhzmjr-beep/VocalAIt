
cmake_minimum_required(VERSION 3.22)

project(VocalAIStudio
    VERSION 0.1.0
    DESCRIPTION "Vocal AI Studio"
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(VAS_FETCH_JUCE "Descargar JUCE automaticamente" ON)

# ------------------------------------------------------------
# JUCE
# ------------------------------------------------------------

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/JUCE/CMakeLists.txt")

    add_subdirectory(JUCE)

elseif(VAS_FETCH_JUCE)

    include(FetchContent)

    FetchContent_Declare(
        JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.10
        GIT_SHALLOW TRUE
    )

    FetchContent_MakeAvailable(JUCE)

else()

    message(FATAL_ERROR "No se encuentra JUCE.")

endif()

# ------------------------------------------------------------
# FORMATOS
# ------------------------------------------------------------

if(APPLE)
    set(VAS_FORMATS VST3 AU)
else()
    set(VAS_FORMATS VST3)
endif()

# ------------------------------------------------------------
# PLUGIN
# ------------------------------------------------------------

juce_add_plugin(VocalAIStudio
    COMPANY_NAME "Vocal AI Studio"
    BUNDLE_ID "studio.vocalai.vocalaistudio"
    PLUGIN_MANUFACTURER_CODE VoAI
    PLUGIN_CODE VcAI
    PRODUCT_NAME "Vocal AI Studio"
    DESCRIPTION "Procesador vocal"
    FORMATS ${VAS_FORMATS}
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD FALSE
)

# IMPORTANTE:
# Genera JuceHeader.h para los archivos fuente que lo incluyen.
juce_generate_juce_header(VocalAIStudio)

# ------------------------------------------------------------
# CODIGO FUENTE
# ------------------------------------------------------------

target_sources(VocalAIStudio PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
)

# ------------------------------------------------------------
# DEFINICIONES
# ------------------------------------------------------------

target_compile_definitions(VocalAIStudio
    PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

# ------------------------------------------------------------
# MODULOS JUCE
# ------------------------------------------------------------

target_link_libraries(VocalAIStudio
    PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_dsp

    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

# ------------------------------------------------------------
# PRUEBAS OPCIONALES
# ------------------------------------------------------------

include(CTest)

if(BUILD_TESTING
   AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Tests/check_source.py")

    find_package(Python3 COMPONENTS Interpreter QUIET)

    if(Python3_Interpreter_FOUND)
        add_test(
            NAME source_contracts
            COMMAND
                "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/Tests/check_source.py"
                "${CMAKE_CURRENT_SOURCE_DIR}"
        )
    endif()

endif()

message(STATUS "Vocal AI Studio configurado")
message(STATUS "Formatos: ${VAS_FORMATS}")
