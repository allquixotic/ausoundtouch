cmake_minimum_required(VERSION 3.22)
project(AUSoundTouch VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Option to use system SoundTouch or fetch from source
option(USE_SYSTEM_SOUNDTOUCH "Use system-installed SoundTouch instead of fetching from source" OFF)

if(USE_SYSTEM_SOUNDTOUCH)
    # Find required packages
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SOUNDTOUCH REQUIRED soundtouch)
    
    # Debug SoundTouch variables
    message(STATUS "SOUNDTOUCH_INCLUDE_DIRS: ${SOUNDTOUCH_INCLUDE_DIRS}")
    message(STATUS "SOUNDTOUCH_LIBRARIES: ${SOUNDTOUCH_LIBRARIES}")
    message(STATUS "SOUNDTOUCH_LIBRARY_DIRS: ${SOUNDTOUCH_LIBRARY_DIRS}")
    
    # Fix include path - remove the /soundtouch suffix
    string(REPLACE "/soundtouch" "" SOUNDTOUCH_INCLUDE_DIRS_FIXED "${SOUNDTOUCH_INCLUDE_DIRS}")
    message(STATUS "Fixed SOUNDTOUCH_INCLUDE_DIRS: ${SOUNDTOUCH_INCLUDE_DIRS_FIXED}")
else()
    # Function to get latest SoundTouch tag from Codeberg
    function(get_latest_soundtouch_tag OUTPUT_VAR)
        # Check if we have a cached version from today
        set(SOUNDTOUCH_VERSION_CACHE_FILE "${CMAKE_BINARY_DIR}/soundtouch_version_cache.txt")
        
        if(EXISTS ${SOUNDTOUCH_VERSION_CACHE_FILE})
            file(TIMESTAMP ${SOUNDTOUCH_VERSION_CACHE_FILE} cache_time "%Y%m%d")
            string(TIMESTAMP current_time "%Y%m%d")
            if(cache_time STREQUAL current_time)
                file(READ ${SOUNDTOUCH_VERSION_CACHE_FILE} CACHED_SOUNDTOUCH_VERSION)
                set(${OUTPUT_VAR} ${CACHED_SOUNDTOUCH_VERSION} PARENT_SCOPE)
                message(STATUS "Using cached SoundTouch version: ${CACHED_SOUNDTOUCH_VERSION}")
                return()
            endif()
        endif()
        
        # Query Codeberg API for SoundTouch tags
        message(STATUS "Fetching latest SoundTouch release...")
        file(DOWNLOAD 
            "https://codeberg.org/api/v1/repos/soundtouch/soundtouch/tags?limit=10"
            "${CMAKE_BINARY_DIR}/soundtouch_tags.json"
            TIMEOUT 10
            STATUS download_status
            INACTIVITY_TIMEOUT 10
        )
        
        list(GET download_status 0 status_code)
        if(NOT status_code EQUAL 0)
            message(WARNING "Failed to fetch SoundTouch tags from Codeberg, using default version 2.4.0")
            set(${OUTPUT_VAR} "2.4.0" PARENT_SCOPE)
            return()
        endif()
        
        # Read the JSON file
        file(READ "${CMAKE_BINARY_DIR}/soundtouch_tags.json" tags_json)
        
        # Parse version tags (looking for simple version numbers like 2.4.0)
        string(REGEX MATCHALL "\"name\"[ ]*:[ ]*\"[0-9]+\\.[0-9]+\\.[0-9]+\"" version_entries "${tags_json}")
        
        set(latest_version "0.0.0")
        set(latest_tag "")
        
        foreach(entry ${version_entries})
            # Extract version number
            string(REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+" version_string "${entry}")
            
            if(version_string AND version_string VERSION_GREATER latest_version)
                set(latest_version ${version_string})
                set(latest_tag ${version_string})
            endif()
        endforeach()
        
        if(latest_tag STREQUAL "")
            message(WARNING "No SoundTouch version tags found, using default version 2.4.0")
            set(${OUTPUT_VAR} "2.4.0" PARENT_SCOPE)
            return()
        endif()
        
        message(STATUS "Found latest SoundTouch version: ${latest_tag}")
        file(WRITE ${SOUNDTOUCH_VERSION_CACHE_FILE} ${latest_tag})
        set(${OUTPUT_VAR} ${latest_tag} PARENT_SCOPE)
    endfunction()
    
    # Get the latest SoundTouch tag or use override
    if(NOT DEFINED SOUNDTOUCH_VERSION)
        get_latest_soundtouch_tag(SOUNDTOUCH_VERSION)
    endif()
    
    # Fetch SoundTouch using FetchContent
    include(FetchContent)
    FetchContent_Declare(
        SoundTouch
        GIT_REPOSITORY https://codeberg.org/soundtouch/soundtouch.git
        GIT_TAG        ${SOUNDTOUCH_VERSION}
        GIT_SHALLOW    TRUE
        GIT_PROGRESS   TRUE
        SOURCE_DIR     "${CMAKE_BINARY_DIR}/_deps/soundtouch-src"
    )
    
    # Disable SoundTouch extras we don't need
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build static SoundTouch library")
    set(SOUNDTOUCH_DLL OFF CACHE BOOL "Build SoundTouch DLL")
    
    FetchContent_MakeAvailable(SoundTouch)
    
    # Set up include and library paths for fetched SoundTouch
    set(SOUNDTOUCH_INCLUDE_DIRS_FIXED "${CMAKE_BINARY_DIR}/_deps/soundtouch-src/include")
    set(SOUNDTOUCH_LIBRARIES SoundTouch)
    
    message(STATUS "Using fetched SoundTouch ${SOUNDTOUCH_VERSION}")
    message(STATUS "SoundTouch include: ${SOUNDTOUCH_INCLUDE_DIRS_FIXED}")
endif()

# Function to get latest JUCE 8.x tag
function(get_latest_juce_8x_tag OUTPUT_VAR)
    # Check if we have a cached version from today
    set(JUCE_VERSION_CACHE_FILE "${CMAKE_BINARY_DIR}/juce_version_cache.txt")
    set(UPDATE_JUCE_VERSION TRUE)
    
    if(EXISTS ${JUCE_VERSION_CACHE_FILE})
        file(TIMESTAMP ${JUCE_VERSION_CACHE_FILE} cache_time "%Y%m%d")
        string(TIMESTAMP current_time "%Y%m%d")
        if(cache_time STREQUAL current_time)
            file(READ ${JUCE_VERSION_CACHE_FILE} CACHED_JUCE_VERSION)
            set(${OUTPUT_VAR} ${CACHED_JUCE_VERSION} PARENT_SCOPE)
            message(STATUS "Using cached JUCE version: ${CACHED_JUCE_VERSION}")
            return()
        endif()
    endif()
    
    # Query GitHub API for JUCE tags
    message(STATUS "Fetching latest JUCE 8.x release...")
    file(DOWNLOAD 
        "https://api.github.com/repos/juce-framework/JUCE/tags?per_page=100"
        "${CMAKE_BINARY_DIR}/juce_tags.json"
        TIMEOUT 10
        STATUS download_status
        INACTIVITY_TIMEOUT 10
    )
    
    list(GET download_status 0 status_code)
    if(NOT status_code EQUAL 0)
        message(WARNING "Failed to fetch JUCE tags from GitHub, using default version 8.0.4")
        set(${OUTPUT_VAR} "8.0.4" PARENT_SCOPE)
        return()
    endif()
    
    # Read the JSON file
    file(READ "${CMAKE_BINARY_DIR}/juce_tags.json" tags_json)
    
    # Simple regex parsing for version tags (CMake's JSON support is limited)
    string(REGEX MATCHALL "\"name\"[ ]*:[ ]*\"8\\.[0-9]+\\.[0-9]+\"" version_entries "${tags_json}")
    
    set(latest_version "0.0.0")
    set(latest_tag "")
    
    foreach(entry ${version_entries})
        # Extract version number
        string(REGEX MATCH "8\\.[0-9]+\\.[0-9]+" version_string "${entry}")
        
        if(version_string AND version_string VERSION_GREATER latest_version)
            set(latest_version ${version_string})
            set(latest_tag ${version_string})
        endif()
    endforeach()
    
    if(latest_tag STREQUAL "")
        message(WARNING "No JUCE 8.x tags found, using default version 8.0.4")
        set(${OUTPUT_VAR} "8.0.4" PARENT_SCOPE)
        return()
    endif()
    
    message(STATUS "Found latest JUCE 8.x version: ${latest_tag}")
    file(WRITE ${JUCE_VERSION_CACHE_FILE} ${latest_tag})
    set(${OUTPUT_VAR} ${latest_tag} PARENT_SCOPE)
endfunction()

# Get the latest JUCE 8.x tag or use override
if(NOT DEFINED JUCE_VERSION)
    get_latest_juce_8x_tag(JUCE_VERSION)
endif()

# Fetch JUCE using FetchContent
include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        ${JUCE_VERSION}
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE
    SOURCE_DIR     "${CMAKE_BINARY_DIR}/_deps/juce-src"
)

# Disable JUCE extras we don't need
set(JUCE_BUILD_EXTRAS OFF CACHE BOOL "Build JUCE Extras")
set(JUCE_BUILD_EXAMPLES OFF CACHE BOOL "Build JUCE Examples")

FetchContent_MakeAvailable(JUCE)

# Plugin target
juce_add_plugin(AUSoundTouch
    COMPANY_NAME "Sean McNamara"
    COMPANY_WEBSITE "https://github.com/allquixotic"
    COMPANY_EMAIL "smcnam@gmail.com"
    PLUGIN_MANUFACTURER_CODE SnMc
    PLUGIN_CODE aust
    FORMATS AU AUv3
    PRODUCT_NAME "AUSoundTouch"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    AU_MAIN_TYPE kAudioUnitType_Effect
    VST3_CATEGORIES Fx Pitch
    BUNDLE_ID com.github.allquixotic.ausoundtouch
    MICROPHONE_PERMISSION_ENABLED FALSE
    CAMERA_PERMISSION_ENABLED FALSE
)

# Generate JUCE header
juce_generate_juce_header(AUSoundTouch)

# Add sources
target_sources(AUSoundTouch
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/SoundTouchWrapper.cpp
)

# Include directories
target_include_directories(AUSoundTouch
    PRIVATE
        ${SOUNDTOUCH_INCLUDE_DIRS_FIXED}
)

# Link directories (needed for SoundTouch when using system version)
if(USE_SYSTEM_SOUNDTOUCH)
    target_link_directories(AUSoundTouch
        PRIVATE
            ${SOUNDTOUCH_LIBRARY_DIRS}
    )
endif()

# Link libraries
target_link_libraries(AUSoundTouch
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
        ${SOUNDTOUCH_LIBRARIES}
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# AU target also needs the link directory when using system SoundTouch
if(USE_SYSTEM_SOUNDTOUCH)
    get_target_property(AU_TARGET_NAME AUSoundTouch JUCE_AU_COPY_DIR)
    if(TARGET AUSoundTouch_AU)
        target_link_directories(AUSoundTouch_AU
            PRIVATE
                ${SOUNDTOUCH_LIBRARY_DIRS}
        )
    endif()
endif()

# Compile definitions
target_compile_definitions(AUSoundTouch
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        $<$<CONFIG:Debug>:DEBUG=1>
        $<$<CONFIG:Debug>:_DEBUG=1>
        $<$<CONFIG:Release>:NDEBUG=1>
)

# Unit Tests
juce_add_console_app(AUSoundTouchTests
    PRODUCT_NAME "AUSoundTouch Unit Tests"
    COMPANY_NAME "Sean McNamara"
)

# Generate JUCE header for tests
juce_generate_juce_header(AUSoundTouchTests)

target_sources(AUSoundTouchTests
    PRIVATE
        Tests/Unit/Main.cpp
        Tests/Unit/ParameterConversionTests.cpp
        Tests/Unit/SoundTouchWrapperTests.cpp
        Tests/Unit/AudioProcessorTests.cpp
        Tests/Unit/ParameterFormattingTests.cpp
        Source/SoundTouchWrapper.cpp
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
)

target_include_directories(AUSoundTouchTests
    PRIVATE
        Source
        ${SOUNDTOUCH_INCLUDE_DIRS_FIXED}
)

# Add compile flags for SoundTouch
target_compile_options(AUSoundTouchTests PRIVATE ${SOUNDTOUCH_CFLAGS_OTHER})

# Add plugin definitions for tests
target_compile_definitions(AUSoundTouchTests
    PRIVATE
        JucePlugin_Name="AUSoundTouch"
        JucePlugin_Desc="Audio pitch/tempo/speed processor"
        JucePlugin_Manufacturer="Sean McNamara"
        JucePlugin_ManufacturerWebsite="https://github.com/allquixotic"
        JucePlugin_ManufacturerEmail="smcnam@gmail.com"
        JucePlugin_ManufacturerCode=0x59524344
        JucePlugin_PluginCode=0x41535463
        JucePlugin_IsSynth=0
        JucePlugin_WantsMidiInput=0
        JucePlugin_ProducesMidiOutput=0
        JucePlugin_IsMidiEffect=0
        JucePlugin_EditorRequiresKeyboardFocus=0
        JucePlugin_Version=1.0.0
        JucePlugin_VersionCode=0x10000
        JucePlugin_VersionString="1.0.0"
        $<$<CONFIG:Debug>:DEBUG=1>
        $<$<CONFIG:Debug>:_DEBUG=1>
        $<$<CONFIG:Release>:NDEBUG=1>
)

if(USE_SYSTEM_SOUNDTOUCH)
    target_link_directories(AUSoundTouchTests
        PRIVATE
            ${SOUNDTOUCH_LIBRARY_DIRS}
    )
endif()

target_link_libraries(AUSoundTouchTests
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
        ${SOUNDTOUCH_LIBRARIES}
    PUBLIC
        juce::juce_recommended_config_flags
)

# Enable testing
enable_testing()
add_test(NAME UnitTests COMMAND AUSoundTouchTests)

# Functional Tests
juce_add_console_app(AUSoundTouchFunctionalTests
    PRODUCT_NAME "AUSoundTouch Functional Tests"
    COMPANY_NAME "Sean McNamara"
)

# Generate JUCE header for functional tests
juce_generate_juce_header(AUSoundTouchFunctionalTests)

target_sources(AUSoundTouchFunctionalTests
    PRIVATE
        Tests/Functional/Main.cpp
        Tests/Functional/PluginValidationTests.cpp
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/SoundTouchWrapper.cpp
)

target_include_directories(AUSoundTouchFunctionalTests
    PRIVATE
        Source
        ${SOUNDTOUCH_INCLUDE_DIRS_FIXED}
)

target_compile_definitions(AUSoundTouchFunctionalTests
    PRIVATE
        JucePlugin_Name="AUSoundTouch"
        JucePlugin_Desc="Audio pitch/tempo/speed processor"
        JucePlugin_Manufacturer="Sean McNamara"
        JucePlugin_ManufacturerWebsite="https://github.com/allquixotic"
        JucePlugin_ManufacturerEmail="smcnam@gmail.com"
        JucePlugin_ManufacturerCode=0x59524344
        JucePlugin_PluginCode=0x41535463
        JucePlugin_IsSynth=0
        JucePlugin_WantsMidiInput=0
        JucePlugin_ProducesMidiOutput=0
        JucePlugin_IsMidiEffect=0
        JucePlugin_EditorRequiresKeyboardFocus=0
        JucePlugin_Version=1.0.0
        JucePlugin_VersionCode=0x10000
        JucePlugin_VersionString="1.0.0"
        $<$<CONFIG:Debug>:DEBUG=1>
        $<$<CONFIG:Debug>:_DEBUG=1>
        $<$<CONFIG:Release>:NDEBUG=1>
)

if(USE_SYSTEM_SOUNDTOUCH)
    target_link_directories(AUSoundTouchFunctionalTests
        PRIVATE
            ${SOUNDTOUCH_LIBRARY_DIRS}
    )
endif()

target_link_libraries(AUSoundTouchFunctionalTests
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
        ${SOUNDTOUCH_LIBRARIES}
    PUBLIC
        juce::juce_recommended_config_flags
)

add_test(NAME FunctionalTests COMMAND AUSoundTouchFunctionalTests)

# Pitch Shift Validation Test (standalone executable)
juce_add_console_app(PitchShiftValidationTest
    PRODUCT_NAME "Pitch Shift Validation Test"
    COMPANY_NAME "Sean McNamara"
)

# Generate JUCE header for pitch shift test
juce_generate_juce_header(PitchShiftValidationTest)

target_sources(PitchShiftValidationTest
    PRIVATE
        Tests/Functional/PitchShiftValidationTest.cpp
)

target_compile_definitions(PitchShiftValidationTest
    PRIVATE
        JUCE_PLUGINHOST_AU=1
        JUCE_PLUGINHOST_VST3=1
        $<$<CONFIG:Debug>:DEBUG=1>
        $<$<CONFIG:Debug>:_DEBUG=1>
        $<$<CONFIG:Release>:NDEBUG=1>
)

target_link_libraries(PitchShiftValidationTest
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_processors
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
)

add_test(NAME PitchShiftValidation COMMAND PitchShiftValidationTest)