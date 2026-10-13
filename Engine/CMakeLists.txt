add_library(alors_pattern STATIC
    Pattern/Fraction.h
    Pattern/TimeSpan.h
    Pattern/Event.h
    Pattern/Pattern.h
    Pattern/Pattern.cpp)
target_include_directories(alors_pattern PUBLIC Pattern)

add_library(alors_engine STATIC
    Audio/AudioEngine.h
    Audio/AudioEngine.cpp
    Audio/BuiltInKit.h
    Audio/BuiltInKit.cpp
    Audio/SampleBank.h
    Audio/SampleBank.cpp
    Audio/SamplerVoices.h
    Audio/SamplerVoices.cpp
    Audio/Scheduler.h
    Audio/Scheduler.cpp
    Audio/SpscQueue.h)
target_include_directories(alors_engine PUBLIC Audio)
target_link_libraries(alors_engine PUBLIC alors_pattern alors_juce)

if(ONDANSE_BUILD_TESTS)
    add_executable(alors_engine_tests tests/PatternTests.cpp)
    target_link_libraries(alors_engine_tests PRIVATE alors_pattern Catch2::Catch2WithMain)

    add_executable(alors_audio_tests tests/EngineTests.cpp)
    target_link_libraries(alors_audio_tests PRIVATE alors_engine Catch2::Catch2WithMain)

    include(Catch)
    catch_discover_tests(alors_engine_tests)
    catch_discover_tests(alors_audio_tests)
endif()
