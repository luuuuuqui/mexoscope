file(GLOB_RECURSE PLUGINFILES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*)

juce_add_plugin(mexoscope
        # VERSION ...                               # Set this if the plugin version is different to the project version
        # ICON_BIG ...                              # ICON_* arguments specify a path to an image file to use as an icon for the Standalone
        # ICON_SMALL ...
        COMPANY_NAME "Kwwala"                          # Specify the name of the plugin's author
        BUNDLE_ID "com.kwwala.mexoscope"
        COMPANY_WEBSITE "https://github.com/hollance/sMexoscope"
        IS_SYNTH FALSE                       # Is this a synth or an effect?
        NEEDS_MIDI_INPUT FALSE               # Does the plugin need midi input?
        NEEDS_MIDI_OUTPUT FALSE              # Does the plugin need midi output?
        IS_MIDI_EFFECT FALSE                 # Is this plugin a MIDI effect?
        EDITOR_WANTS_KEYBOARD_FOCUS FALSE    # Does the editor need keyboard focus?
        # COPY_PLUGIN_AFTER_BUILD TRUE/FALSE        # Should the plugin be installed to a default location after building?
        PLUGIN_MANUFACTURER_CODE Kww4               # A four-character manufacturer id with at least one upper-case character
        PLUGIN_CODE Mexo                            # A unique four-character plugin id with exactly one upper-case character
        # GarageBand 10.3 requires the first letter to be upper-case, and the remaining letters to be lower-case
        FORMATS AU VST3 LV2 Standalone                  # The formats to build. Other valid formats are: AAX Unity VST AU AUv3
        PRODUCT_NAME "mexoscope")        # The name of the final executable, which can differ from the target name


juce_generate_juce_header(mexoscope)

target_sources(mexoscope PUBLIC ${PLUGINFILES})

target_compile_definitions(mexoscope
        PUBLIC
        # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_plugin` call
        JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_plugin` call
        JUCE_VST3_CAN_REPLACE_VST2=0)

target_include_directories(mexoscope PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(mexoscope
        PUBLIC
        juce::juce_audio_utils
        juce::juce_dsp

        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)


clap_juce_extensions_plugin(TARGET mexoscope
        CLAP_ID "com.kwwala.mexoscope"
        CLAP_FEATURES CLAP_PLUGIN_FEATURE_AUDIO_EFFECT "oscilloscope"
)

set_target_properties(
        "mexoscope_CLAP"
        PROPERTIES
        XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER
        "com.kwwala.mexoscope"
)

if (MSVC)
    target_compile_options(mexoscope PRIVATE "/W4")
    target_compile_options(mexoscope PUBLIC "/EHsc")
else ()
    target_compile_options(mexoscope PRIVATE "-Wall")
    target_compile_options(mexoscope PRIVATE "-Wextra")
    target_compile_options(mexoscope PRIVATE "-fpermissive")
endif ()
