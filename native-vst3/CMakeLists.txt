cmake_minimum_required(VERSION 3.22)
project(SpaceInfinity VERSION 0.6.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
include(FetchContent)
FetchContent_Declare(juce GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.7)
FetchContent_MakeAvailable(juce)
juce_add_plugin(SpaceInfinity COMPANY_NAME "Orbital Audio" BUNDLE_ID "com.orbitalaudio.spaceinfinity" PLUGIN_MANUFACTURER_CODE Orbt PLUGIN_CODE SInf FORMATS VST3 Standalone PRODUCT_NAME "SPACE INFINITY" IS_SYNTH TRUE NEEDS_MIDI_INPUT TRUE NEEDS_MIDI_OUTPUT FALSE COPY_PLUGIN_AFTER_BUILD FALSE)
file(GLOB TRANCE_SAMPLE_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Samples/Trance/*.wav")
juce_add_binary_data(SpaceInfinitySamples SOURCES ${TRANCE_SAMPLE_FILES}
    Samples/Orbital_Choir_Ah_Chord_F2.wav
    Samples/Ethereal_Choir_Ooh_C3.wav
    Samples/Choir_claudia_ah.wav
    Samples/Choir_claudia_ih.wav
    Samples/Choir_claudia_oh.wav
    Samples/Choir_mirko_ah.wav
    Samples/Choir_mirko_oh.wav
    Samples/Choir_sven_ah.wav
    Samples/Choir_sven_ih.wav
    Samples/Choir_sven_oh.wav
    Samples/VCSL_Bowed_Psaltery.wav
    Samples/VCSL_FM_Clavi.wav
    Samples/VCSL_FM_Piano.wav
    Samples/VCSL_Glockenspiel.wav
    Samples/VCSL_Lute_Pluck.wav
    Samples/VCSL_Marimba.wav
    Samples/VCSL_Recorder_Stab.wav
    Samples/VCSL_Vibraphone.wav
)
file(GLOB FACTORY_OVERRIDES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Presets/Overrides/*.sipreset")
juce_add_binary_data(SpaceInfinityFactoryBank HEADER_NAME FactoryBankData.h NAMESPACE FactoryBankData SOURCES Presets/TranceFactoryBank.json ${FACTORY_OVERRIDES})
juce_generate_juce_header(SpaceInfinity)
juce_add_binary_data(SpaceInfinityArtwork HEADER_NAME Artwork.h NAMESPACE Artwork SOURCES Assets/InfinityLogo.png)
target_sources(SpaceInfinity PRIVATE Source/PluginProcessor.cpp Source/PluginEditor.cpp Source/ExtendedProcessor.cpp Source/StudioProcessor.cpp Source/StudioRack.cpp)
target_compile_definitions(SpaceInfinity PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(SpaceInfinity PRIVATE SpaceInfinityFactoryBank SpaceInfinityArtwork SpaceInfinitySamples juce::juce_audio_utils juce::juce_dsp juce::juce_recommended_config_flags juce::juce_recommended_lto_flags juce::juce_recommended_warning_flags)

option(SPACE_INFINITY_BUILD_TESTS "Build native smoke tests on other desktop platforms" OFF)
if(WIN32 OR SPACE_INFINITY_BUILD_TESTS)
    juce_add_console_app(SpaceInfinitySmoke PRODUCT_NAME "Space Infinity Smoke")
    juce_generate_juce_header(SpaceInfinitySmoke)
    target_sources(SpaceInfinitySmoke PRIVATE Tests/Smoke.cpp Tests/CreativeSmoke.cpp Tests/UsabilitySmoke.cpp Tests/ExtendedSmoke.cpp Tests/PresetSmoke.cpp Tests/StudioSmoke.cpp Tests/TranceBankSmoke.cpp Tests/WorkflowSmoke.cpp Tests/RhythmWorkflowSmoke.cpp Tests/DualLfoSmoke.cpp Tests/OscillatorWorkflowSmoke.cpp Tests/AnalogueExpansionSmoke.cpp)
    target_compile_definitions(SpaceInfinitySmoke PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)
    target_link_libraries(SpaceInfinitySmoke PRIVATE SpaceInfinity SpaceInfinitySamples juce::juce_audio_utils juce::juce_dsp)
    add_executable(SpaceInfinityAnalogueProbe Tests/AnalogueProbe.cpp)
    add_executable(SpaceInfinityExtendedCore Tests/ExtendedCore.cpp)
    if(MSVC)
        target_compile_options(SpaceInfinityExtendedCore PRIVATE /UNDEBUG)
    else()
        target_compile_options(SpaceInfinityExtendedCore PRIVATE -UNDEBUG)
    endif()
    enable_testing()
    add_test(NAME SpaceInfinityExtendedCore COMMAND SpaceInfinityExtendedCore)
    add_test(NAME SpaceInfinityCreativeWorkflow COMMAND SpaceInfinitySmoke --creative-tests)
    add_test(NAME SpaceInfinityUsability COMMAND SpaceInfinitySmoke --usability-tests)
    add_test(NAME SpaceInfinitySmoke COMMAND SpaceInfinitySmoke)
    add_test(NAME SpaceInfinityHardwareUI COMMAND SpaceInfinitySmoke --ui-only)
    add_test(NAME SpaceInfinityPresetManagement COMMAND SpaceInfinitySmoke --preset-tests)
    add_test(NAME SpaceInfinityExtendedSmoke COMMAND SpaceInfinitySmoke --extended-tests)
    add_test(NAME SpaceInfinityAnalogueExpansion COMMAND SpaceInfinitySmoke --analogue-expansion-tests)
    add_test(NAME SpaceInfinityOscillatorWorkflow COMMAND SpaceInfinitySmoke --oscillator-workflow-tests)
    add_test(NAME SpaceInfinityDualLFO COMMAND SpaceInfinitySmoke --dual-lfo-tests)
    add_test(NAME SpaceInfinityRhythmWorkflow COMMAND SpaceInfinitySmoke --rhythm-workflow-tests)
    add_test(NAME SpaceInfinityWorkflow COMMAND SpaceInfinitySmoke --workflow-tests)
    add_test(NAME SpaceInfinityStudioSmoke COMMAND SpaceInfinitySmoke --studio-tests)
    add_test(NAME SpaceInfinityGrooveBank COMMAND SpaceInfinitySmoke --groove-tests)
    add_test(NAME SpaceInfinityLeads COMMAND SpaceInfinitySmoke --lead-tests)
    add_test(NAME SpaceInfinityAirwavePad COMMAND SpaceInfinitySmoke --pad-tests)
    add_test(NAME SpaceInfinityAnalogueProbe COMMAND SpaceInfinityAnalogueProbe)
endif()

