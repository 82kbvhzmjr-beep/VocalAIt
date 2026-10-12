
cmake_minimum_required(VERSION 3.22)

project(VocalAIStudio
    VERSION 0.1.0
    DESCRIPTION "Vocal AI Studio - Procesador vocal para macOS"
    LANGUAGES C CXX
)

# ============================================================
# CONFIGURACION GENERAL
# ============================================================

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(
    VAS_FETCH_JUCE
    "Descargar JUCE automaticamente si no esta instalado"
    ON
)

# ============================================================
# CONFIGURACION DE MACOS
# ============================================================

if(APPLE)
    if(NOT CMAKE_OSX_DEPLOYMENT_TARGET)
        set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0"
            CACHE STRING "Version minima de macOS" FORCE)
    endif()

    message(STATUS "Configurando Vocal AI Studio para macOS")
    message(STATUS "Arquitecturas: ${CMAKE_OSX_ARCHITECTURES}")
endif()

# ============================================================
# JUCE
# ============================================================

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/JUCE/CMakeLists.txt")

    message(STATUS "Utilizando JUCE local")
    add_subdirectory(JUCE)

elseif(VAS_FETCH_JUCE)

    message(STATUS "Descargando JUCE 8.0.10")

    include(FetchContent)

    FetchContent_Declare(
        JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.10
        GIT_SHALLOW TRUE
    )

    FetchContent_MakeAvailable(JUCE)

else()

    message(FATAL_ERROR
        "No se encuentra JUCE. Activa VAS_FETCH_JUCE o instala JUCE en ./JUCE."
    )

endif()

# ============================================================
# FORMATOS DEL PLUGIN
# ============================================================

if(APPLE)

    set(VAS_PLUGIN_FORMATS VST3 AU)

else()

    set(VAS_PLUGIN_FORMATS VST3)

endif()

# ============================================================
# DEFINICION DEL PLUGIN
# ============================================================

juce_add_plugin(VocalAIStudio

    COMPANY_NAME "Vocal AI Studio"

    BUNDLE_ID "studio.vocalai.vocal-ai-studio"

    PLUGIN_MANUFACTURER_CODE VoAI

    PLUGIN_CODE VcAI

    PRODUCT_NAME "Vocal AI Studio"

    DESCRIPTION "Procesador vocal minimalista con herramientas inteligentes"

    FORMATS ${VAS_PLUGIN_FORMATS}

    IS_SYNTH FALSE

    NEEDS_MIDI_INPUT FALSE

    NEEDS_MIDI_OUTPUT FALSE

    IS_MIDI_EFFECT FALSE

    EDITOR_WANTS_KEYBOARD_FOCUS TRUE

    COPY_PLUGIN_AFTER_BUILD FALSE

)

# ============================================================
# CODIGO FUENTE
# ============================================================

target_sources(VocalAIStudio PRIVATE

    Source/PluginProcessor.cpp
    Source/PluginProcessor.h

    Source/PluginEditor.cpp
    Source/PluginEditor.h

)

# ============================================================
# CONFIGURACION DE C++
# ============================================================

target_compile_features(
    VocalAIStudio
    PRIVATE
    cxx_std_17
)

target_compile_definitions(
    VocalAIStudio
    PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

# ============================================================
# MODULOS DE AUDIO
# ============================================================

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

# ============================================================
# PRUEBAS AUTOMATIZADAS
# ============================================================

include(CTest)

find_package(
    Python3
    COMPONENTS Interpreter
    QUIET
)

if(BUILD_TESTING AND Python3_Interpreter_FOUND)

    add_test(
        NAME source_contracts
        COMMAND
            "${Python3_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/Tests/check_source.py"
            "${CMAKE_CURRENT_SOURCE_DIR}"
    )

endif()

# ============================================================
# INFORMACION FINAL
# ============================================================

message(STATUS "======================================")
message(STATUS "Vocal AI Studio")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "Formatos: ${VAS_PLUGIN_FORMATS}")
message(STATUS "C++: ${CMAKE_CXX_STANDARD}")
message(STATUS "======================================")
