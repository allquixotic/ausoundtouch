# AUSoundTouch Developer Documentation

This document provides comprehensive technical information for developers working on AUSoundTouch.

## Project Overview

AUSoundTouch is an AudioUnit v3 plugin for macOS that integrates the SoundTouch audio processing library to provide real-time pitch, tempo, and speed manipulation. Built with JUCE 8.0.8 and modern C++20, it offers professional-quality time-stretching and pitch-shifting capabilities.

## Architecture

### Core Components

1. **SoundTouchWrapper** (`Source/SoundTouchWrapper.{h,cpp}`)
   - Encapsulates the SoundTouch library
   - Handles audio buffer processing
   - Manages parameter conversions (semitones ↔ native, percentage ↔ native)
   - Reports latency for host compensation

2. **PluginProcessor** (`Source/PluginProcessor.{h,cpp}`)
   - JUCE AudioProcessor implementation
   - Parameter management via AudioProcessorValueTreeState
   - Audio buffer routing to SoundTouchWrapper
   - State save/restore functionality

3. **PluginEditor** (`Source/PluginEditor.{h,cpp}`)
   - GUI implementation with rotary sliders
   - Parameter attachments for host automation
   - Custom value formatting with ± signs

### Parameter System

- **Pitch**: -39.8 to +39.8 semitones (native: 0.1-10.0)
- **Tempo**: -90% to +900% (native: 0.1-10.0)
- **Speed**: -90% to +900% (native: 0.1-10.0)

Conversion formulas:
```cpp
// Pitch: semitones to native multiplier
native = exp(semitones * ln(2)/12)

// Tempo/Speed: percentage to native multiplier
native = 1.0 + (percentage / 100.0)
```

## Development Environment Setup

### Prerequisites

1. **macOS Sonoma 15.x+** (required for development)
2. **Xcode 15+** with command line tools
3. **CMake 3.22+** (`brew install cmake`)
4. **Ninja** build system (`brew install ninja`)
5. **SoundTouch 2.4.0+** (`brew install sound-touch`)
6. **pkg-config** (`brew install pkg-config`)
7. **dotenvx** for secrets management (`brew install dotenvx/brew/dotenvx`)
8. **JUCE 8.0.8** (fetched automatically by CMake)

### Repository Structure

```
/Users/sean/dev/aust/
├── AUSoundTouch/              # Plugin source
│   ├── CMakeLists.txt         # Main CMake configuration
│   ├── Source/                # C++ source files
│   ├── Tests/                 # Unit and functional tests
│   └── todo.md               # Implementation tracking
├── JUCE/                      # JUCE framework
├── Makefile                   # Build automation
├── .env                       # Encrypted secrets (via dotenvx)
└── CLAUDE.md                  # AI assistant context
```

### Initial Setup

1. **Clone the repository**:
   ```bash
   git clone https://github.com/allquixotic/ausoundtouch.git
   cd aust
   ```

2. **Install dependencies**:
   ```bash
   brew install cmake ninja sound-touch pkg-config dotenvx/brew/dotenvx
   ```

3. **Set up environment variables**:
   ```bash
   # Create .env file with:
   CODESIGN_ID="Developer ID Application: Sean McNamara (TEAMID)"
   NOTARY_PROFILE="AC_NOTARY"
   APPLE_EMAIL="smcnam@gmail.com"
   APP_PASSWORD="xxxx-xxxx-xxxx-xxxx"  # App-specific password
   ```

4. **Encrypt the .env file**:
   ```bash
   dotenvx encrypt
   ```

## Build System

The project uses a Makefile wrapper around CMake for convenience. All commands should be run from the top-level directory (`/Users/sean/dev/aust/`).

### Common Build Commands

```bash
# Show all available commands
make help

# Build debug version
make build

# Build release version  
make release

# Run tests
make test

# Install to ~/Library/Audio/Plug-Ins/Components/
make install

# Clean, build, test, and install
make all
```

### Code Signing & Notarization

For distribution, you need an Apple Developer ID:

1. **Setup notarization credentials**:
   ```bash
   make setup-notary
   ```

2. **Create signed & notarized distribution**:
   ```bash
   make dist
   ```

This creates a fully notarized DMG at `AUSoundTouch/build/AUSoundTouch.dmg`.

### Build Configurations

- **Debug**: Includes symbols, assertions, no optimization
- **Release**: Optimized, stripped, ready for distribution
- **CMAKE_BUILD_TYPE**: Can be set via environment variable

## Testing

### Unit Tests

Located in `Tests/Unit/`:
- `ParameterConversionTests.cpp` - Conversion formula validation
- `SoundTouchWrapperTests.cpp` - Audio processing logic
- `AudioProcessorTests.cpp` - Plugin lifecycle, automation

Run with:
```bash
make test      # All tests via CTest
make unit      # Unit tests only
```

### AudioUnit Validation

```bash
make validate  # Runs auval -v aufx ASTc YRCM
```

### Memory Testing

```bash
make leaks     # Check for memory leaks (Debug build)
```

## Development Workflow

### TDD "DO THE THING" Loop

The project follows strict Test-Driven Development:

1. **SELECT GOAL**: Pick a small, testable feature
2. **WRITE TEST**: Create failing unit test
3. **RUN TEST**: Verify failure (red phase)
4. **IMPLEMENT**: Minimal code to pass
5. **RUN TEST**: Verify success (green phase)
6. **REFACTOR**: Clean code/tests
7. **REFLECT**: Review design decisions
8. **COMMIT**: Save test+implementation together

### Quick Development Cycle

```bash
make quick     # Build and run unit tests
make reinstall # Remove old plugin and install new one
```

## Using dotenvx

The project uses dotenvx for secure credential management:

1. **View decrypted values**:
   ```bash
   dotenvx get CODESIGN_ID
   ```

2. **Run commands with env vars**:
   ```bash
   dotenvx run -- echo $CODESIGN_ID
   ```

3. **Update values**:
   ```bash
   dotenvx set KEY=value
   dotenvx encrypt
   ```

## Debugging

### Console Output

Check Audio MIDI Setup.app console for plugin loading issues.

### Common Issues

1. **SoundTouch not found**: Ensure PKG_CONFIG_PATH is set correctly
2. **Signing failures**: Check CODESIGN_ID format includes team ID
3. **Notarization errors**: Run `make setup-notary` to update credentials

## Plugin Metadata

- **Plugin Code**: ASTc
- **Manufacturer Code**: YRCM  
- **Bundle ID**: com.yourcompany.ausoundtouch
- **Category**: Audio Effect (aufx)

## Performance Considerations

- SoundTouch introduces latency (typically 64-512 samples)
- Plugin reports latency to host for compensation
- Processing is optimized for real-time performance
- Buffer sizes up to 2048 samples tested

## Contributing

1. Follow the TDD workflow for all changes
2. Ensure all tests pass before committing
3. Update todo.md with implementation status
4. Run `make validate` before submitting PRs

## Release Process

1. Update version in `CMakeLists.txt`
2. Run full test suite: `make test`
3. Validate with auval: `make validate`
4. Create distribution: `make dist`
5. Test in multiple hosts
6. Tag release and upload DMG to GitHub

## Additional Resources

- [JUCE Documentation](https://docs.juce.com)
- [SoundTouch Documentation](https://www.surina.net/soundtouch/)
- [AudioUnit Programming Guide](https://developer.apple.com/documentation/audiounit)
- [CMake Documentation](https://cmake.org/documentation/)