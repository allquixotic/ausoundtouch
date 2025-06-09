#!/bin/bash
# Check AUSoundTouch dependencies

echo "Checking AUSoundTouch dependencies..."
echo

# Check for SoundTouch
echo -n "SoundTouch library: "
if pkg-config --exists soundtouch 2>/dev/null; then
    VERSION=$(pkg-config --modversion soundtouch)
    echo "✓ Found version $VERSION"
    echo "  Include path: $(pkg-config --cflags soundtouch)"
    echo "  Library path: $(pkg-config --libs soundtouch)"
else
    echo "✗ Not found"
    echo "  Install with: brew install sound-touch"
    exit 1
fi

echo

# Check for CMake
echo -n "CMake: "
if command -v cmake &> /dev/null; then
    VERSION=$(cmake --version | head -n1 | cut -d' ' -f3)
    echo "✓ Found version $VERSION"
else
    echo "✗ Not found"
    echo "  Install with: brew install cmake"
    exit 1
fi

echo

# Check for Ninja
echo -n "Ninja: "
if command -v ninja &> /dev/null; then
    VERSION=$(ninja --version)
    echo "✓ Found version $VERSION"
else
    echo "✗ Not found"
    echo "  Install with: brew install ninja"
    exit 1
fi

echo

# Check for pkg-config
echo -n "pkg-config: "
if command -v pkg-config &> /dev/null; then
    VERSION=$(pkg-config --version)
    echo "✓ Found version $VERSION"
else
    echo "✗ Not found"
    echo "  Install with: brew install pkg-config"
    exit 1
fi

echo

# Check for JUCE
echo -n "JUCE: "
if [ -d "JUCE/modules" ]; then
    echo "✓ Found in ./JUCE"
else
    echo "✗ Not found"
    echo "  JUCE directory missing"
    exit 1
fi

echo
echo "All dependencies found! ✅"
echo
echo "You can now build with: make build"