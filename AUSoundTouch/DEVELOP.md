# AUSoundTouch Development Guide

## ⚠️ CRITICAL: macOS Audio Unit Cache Issue

**Problem**: macOS aggressively caches Audio Units. Simply copying a new build over an existing plugin often doesn't take effect - the system continues using the cached version. This is especially problematic with code-signed release builds.

**Solution**: Always use the Makefile install targets which now:
1. Remove any existing plugin first
2. Sign the build (for release)
3. Install the new version
4. Clear the AU cache by killing AudioComponentRegistrar

**Recommended Workflows**:
```bash
# Debug build and test
make clean && make build && make install && make func

# Release build and test
make clean && make release && make install-release && make func-release

# Using shortcuts
make c && make b && make i && make func     # Debug
make c && make release && make ir && make func-release  # Release
```

**What NOT to do**:
```bash
# This may use cached/old plugin:
cp -R build/.../AUSoundTouch.component ~/Library/Audio/Plug-Ins/Components/

# Even this isn't enough for signed builds:
make release && make sign && make install-release  # Missing clean!
```

## Overview
This document provides detailed development information for AUSoundTouch, including testing frameworks, validation tools, and development workflows.

## Test Architecture

### Test Categories

**Unit Tests** (`Tests/Unit/`):
- Fast, isolated tests for individual components
- Run via `make unit` or `make unit-release`
- Coverage: Parameter conversions, SoundTouch wrapper, processor logic

**Functional Tests** (`Tests/Functional/`):
- Integration tests simulating real plugin host environment
- Run via `make func` or `make func-release`
- Coverage: Plugin loading, host validation, end-to-end processing

**Validation Tests** (Specialized functional tests):
- Advanced signal analysis and automated quality verification
- Audio processing accuracy validation with objective metrics
- Manual verification options for subjective quality assessment

### PitchShiftValidationTest - Advanced Signal Validation

The `PitchShiftValidationTest` is a sophisticated functional test that validates the core pitch shifting functionality using automated signal analysis.

#### Purpose
- **Automated Quality Assurance**: Verifies pitch shifting accuracy without human intervention
- **Regression Detection**: Catches audio processing regressions during development
- **Host Compatibility**: Tests plugin loading and processing in a minimal host environment
- **Manual Verification**: Optional audio playback for subjective quality checks

#### Technical Design

**Signal Generation**:
- Generates 5-second stereo sine wave at 440Hz (A4) and 44.1kHz sample rate
- Uses clean mathematical synthesis for predictable reference signal
- Creates realistic test conditions with proper stereo imaging

**Plugin Hosting**:
- Minimal CLI-based AudioUnit host using JUCE's AudioPluginFormatManager
- Loads AUSoundTouch from standard macOS plugin directory
- Configures plugin parameters (sets pitch to +1 semitone)
- Processes audio in 512-sample blocks like real DAW hosts
- Handles plugin latency compensation automatically

**Signal Analysis Engine**:
```cpp
class SignalAnalyzer {
    // FFT-based frequency analysis with Hann windowing
    float findDominantFrequency(const float* audioData, int numSamples, double sampleRate);
    
    // RMS level calculation for amplitude analysis
    float calculateRMS(const float* audioData, int numSamples);
    
    // Dropout detection using windowed RMS analysis
    bool hasDropouts(const float* audioData, int numSamples, float threshold = 0.01f);
};
```

**Validation Criteria**:
- **Frequency Accuracy**: <1% error from expected pitch-shifted frequency (466.16Hz for +1 semitone)
- **Amplitude Preservation**: Output/input RMS ratio between 0.5-2.0
- **Signal Continuity**: No dropouts or silent regions detected
- **Parabolic Interpolation**: Sub-bin frequency accuracy for precise measurements

#### Usage Examples

**Automated Testing** (CI/CD integration):
```bash
# Build and run validation test
make build
make pitch-validate

# Run as part of test suite
make test  # Includes PitchShiftValidation test
```

**Manual Audio Verification**:
```bash
# Run with audio playback for listening test
make pitch-validate-play

# Or run executable directly with options
./build/PitchShiftValidationTest_artefacts/Debug/PitchShiftValidationTest --play
```

**Custom Plugin Path**:
```bash
# Test specific plugin build
./build/PitchShiftValidationTest_artefacts/Debug/PitchShiftValidationTest /path/to/AUSoundTouch.component
```

#### Output Analysis

**Successful Test Output**:
```
=== AUSoundTouch Pitch Shift Validation Test ===
Plugin path: ~/Library/Audio/Plug-Ins/Components/AUSoundTouch.component
Successfully loaded plugin: AUSoundTouch
Set parameter 'pitch' to 1

Generating test signal: 440 Hz sine wave, 5 seconds
Processing through AUSoundTouch with pitch shift: +1 semitones
Plugin latency: 256 samples

=== Signal Analysis ===
Input signal:
  Dominant frequency: 440.00 Hz (expected: 440.00 Hz)
  RMS level: 0.3536

Output signal (excluding latency):
  Dominant frequency: 466.14 Hz (expected: 466.16 Hz)
  RMS level: 0.3534
  Dropouts detected: NO

=== Validation Results ===
Frequency error: 0.02 Hz (0.00%)
RMS ratio (output/input): 0.999

Test result: PASSED
```

**Failed Test Indicators**:
- Frequency error ≥1%: Pitch shifting algorithm issue
- RMS ratio outside 0.5-2.0 range: Amplitude processing problem  
- Dropouts detected: Buffer underruns or processing interruptions
- Plugin loading failure: Installation or compatibility issue

#### Integration with Development Workflow

**TDD Integration**:
1. **Red Phase**: Test fails when pitch shifting is broken
2. **Green Phase**: Test passes when implementation is correct
3. **Refactor Phase**: Test validates that optimizations don't break functionality

**Performance Monitoring**:
- Tracks plugin latency changes over development iterations
- Validates that optimizations maintain audio quality
- Ensures consistent behavior across debug/release builds

**CI/CD Pipeline**:
- Automated execution in continuous integration
- Objective pass/fail criteria for automated builds
- No user interaction required for validation

#### Dependencies and Requirements

**Build Dependencies**:
- JUCE modules: `audio_utils`, `audio_devices`, `audio_formats`, `audio_processors`, `dsp`
- No external audio libraries required (uses JUCE's built-in FFT)
- Standard C++20 and STL components

**Runtime Requirements**:
- macOS with AudioUnit support
- Installed AUSoundTouch plugin (or path to component)
- Audio device access (for optional playback mode)
- Sufficient CPU for real-time processing simulation

**Audio Analysis Capabilities**:
- FFT size: 4096 samples (configurable)
- Frequency resolution: ~10.8 Hz at 44.1kHz
- Parabolic interpolation for sub-bin accuracy
- Multiple FFT windows averaged for noise reduction
- Hann windowing to reduce spectral leakage

#### Advanced Configuration

**Test Parameters** (modifiable in source):
```cpp
const double sampleRate = 44100.0;           // Test sample rate
const int blockSize = 512;                   // Processing block size
const int durationSeconds = 5;               // Test signal length
const float testFrequency = 440.0f;          // Input frequency (A4)
const float pitchShiftSemitones = 1.0f;      // Pitch shift amount
const float frequencyTolerancePercent = 1.0f; // Accuracy threshold
```

**Analysis Tuning**:
```cpp
SignalAnalyzer analyzer(4096);               // FFT size
float dropoutThreshold = 0.01f;              // RMS threshold for dropout detection
```

This test represents a significant advancement in automated audio plugin validation, providing objective quality metrics that can be integrated into continuous integration workflows while maintaining the option for subjective manual verification.

## Development Best Practices

### Test-Driven Development (TDD)
All features should follow the "DO THE THING" loop:
1. **SELECT GOAL**: Choose testable feature (15-60 min)
2. **WRITE TEST**: Create failing test first
3. **RUN TEST**: Verify red state
4. **IMPLEMENT**: Minimal code to pass
5. **RUN TEST**: Verify green state
6. **REFACTOR**: Clean up code/tests
7. **REFLECT**: Review design decisions
8. **COMMIT**: Save test+implementation together

### Quality Gates
Before committing changes:
- All unit tests pass (`make unit`)
- All functional tests pass (`make func`)
- PitchShiftValidationTest passes (`make pitch-validate`)
- No memory leaks (`make leaks`)
- Plugin validates with auval (`make validate`)

### Performance Validation
- Use PitchShiftValidationTest to monitor latency changes
- Validate that optimizations maintain audio quality
- Test both debug and release builds for performance differences