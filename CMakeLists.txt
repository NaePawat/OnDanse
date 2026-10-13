cmake_minimum_required(VERSION 3.25)

project(OnDanse VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ONDANSE_BUILD_TESTS "Build unit tests" ON)

include(FetchContent)

FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 9.0.4
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)

if(ONDANSE_BUILD_TESTS)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.16.1
        GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(Catch2)
    enable_testing()
endif()

add_library(alors_juce STATIC)
target_link_libraries(alors_juce
    PRIVATE
        juce::juce_audio_utils
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
target_compile_definitions(alors_juce
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_STANDALONE_APPLICATION=1
        JUCE_DISPLAY_SPLASH_SCREEN=0
    INTERFACE
        $<TARGET_PROPERTY:alors_juce,COMPILE_DEFINITIONS>)
target_include_directories(alors_juce
    INTERFACE
        $<TARGET_PROPERTY:alors_juce,INCLUDE_DIRECTORIES>)

add_subdirectory(Engine)
add_subdirectory(Application)
