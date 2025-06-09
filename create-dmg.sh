#!/bin/bash
# Create a user-friendly DMG for AUSoundTouch with a shortcut to the Components folder

set -e

# Configuration
PLUGIN_NAME="AUSoundTouch"
BUILD_DIR="AUSoundTouch/build"
COMPONENT_PATH="${BUILD_DIR}/${PLUGIN_NAME}_artefacts/Release/AU/${PLUGIN_NAME}.component"
DMG_PATH="${BUILD_DIR}/${PLUGIN_NAME}.dmg"
TEMP_DIR="${BUILD_DIR}/dmg-contents"
VOLUME_NAME="${PLUGIN_NAME}"

# Check if the component exists
if [ ! -d "${COMPONENT_PATH}" ]; then
    echo "❌ Error: Component not found at ${COMPONENT_PATH}"
    echo "Run 'make release' first to build the plugin."
    exit 1
fi

echo "Creating distributable DMG with Components folder shortcut..."

# Clean up any existing temp directory and DMG
rm -rf "${TEMP_DIR}"
rm -f "${DMG_PATH}"
mkdir -p "${TEMP_DIR}"

# Copy the component to the temp directory
echo "Copying component..."
cp -R "${COMPONENT_PATH}" "${TEMP_DIR}/"

# Create the Components folder if it doesn't exist (for alias creation)
mkdir -p ~/Library/Audio/Plug-Ins/Components

# Create an alias to the Components folder using osascript
echo "Creating alias to Audio Unit Components folder..."
osascript - "${TEMP_DIR}" <<'EOF' 2>/dev/null || true
on run argv
    set tempFolder to item 1 of argv
    tell application "Finder"
        try
            -- Create alias to Components folder
            set componentsPath to (POSIX file ((POSIX path of (path to home folder)) & "Library/Audio/Plug-Ins/Components/")) as alias
            set tempFolderAlias to POSIX file tempFolder as alias
            make alias file at tempFolderAlias to componentsPath with properties {name:"Components"}
        on error errMsg
            -- Silently fail, we'll use symlink as fallback
        end try
    end tell
end run
EOF

# Check if alias was created
ALIAS_CREATED=false
if [ -e "${TEMP_DIR}/Components" ]; then
    ALIAS_CREATED=true
fi

# Create appropriate installation instructions
if [ "$ALIAS_CREATED" = false ]; then
    echo "Note: Components folder alias could not be created, providing manual instructions..."
    cat > "${TEMP_DIR}/Install Instructions.txt" <<'EOL'
AUSoundTouch Installation
========================

INSTALLATION:
1. Drag the AUSoundTouch.component to your Audio Unit Components folder:
   ~/Library/Audio/Plug-Ins/Components/

   You can open this folder by pressing Cmd+Shift+G in Finder and entering:
   ~/Library/Audio/Plug-Ins/Components/

2. Restart your audio software (DAW)

The plugin will appear in your DAW as:
- Manufacturer: SnMc  
- Effect: AUSoundTouch

For help, visit:
https://github.com/allquixotic/ausoundtouch
EOL
else
    # If alias was created successfully, provide simpler instructions
    cat > "${TEMP_DIR}/Install Instructions.txt" <<'EOL'
AUSoundTouch Installation
========================

EASY INSTALLATION:
Drag AUSoundTouch.component onto the Components folder alias →

MANUAL INSTALLATION:
Copy AUSoundTouch.component to:
~/Library/Audio/Plug-Ins/Components/

After installation, restart your audio software.

The plugin will appear as:
- Manufacturer: SnMc
- Effect: AUSoundTouch

For help: https://github.com/allquixotic/ausoundtouch
EOL
fi

# Create the DMG directly from the folder
echo "Building DMG..."
hdiutil create -volname "${VOLUME_NAME}" \
    -srcfolder "${TEMP_DIR}" \
    -fs HFS+ \
    -format UDRW \
    -ov \
    "${BUILD_DIR}/temp.dmg" > /dev/null

# Mount the DMG to set custom view options
echo "Setting DMG appearance..."
DEVICE=$(hdiutil attach -nobrowse -readwrite "${BUILD_DIR}/temp.dmg" | grep "/dev/disk" | head -1 | awk '{print $1}')
MOUNT_POINT="/Volumes/${VOLUME_NAME}"

# Set custom view options
osascript <<EOF 2>/dev/null || true
tell application "Finder"
    try
        tell disk "${VOLUME_NAME}"
            open
            set current view of container window to icon view
            set toolbar visible of container window to false
            set statusbar visible of container window to false
            set the bounds of container window to {400, 100, 885, 385}
            set viewOptions to the icon view options of container window
            set arrangement of viewOptions to not arranged
            set icon size of viewOptions to 72
            set background color of viewOptions to {58982, 58982, 58982}
            try
                set position of item "${PLUGIN_NAME}.component" of container window to {100, 100}
            end try
            if exists item "Components" of container window then
                try
                    set position of item "Components" of container window to {350, 100}
                end try
            end if
            try
                set position of item "Install Instructions.txt" of container window to {225, 200}
            end try
            close
            open
            delay 1
            close
        end tell
    end try
end tell
EOF

# Unmount
echo "Finalizing DMG..."
hdiutil detach "${DEVICE}" -quiet > /dev/null

# Convert to compressed format
hdiutil convert "${BUILD_DIR}/temp.dmg" -format ULMO -o "${DMG_PATH}" -quiet > /dev/null

# Clean up
rm -f "${BUILD_DIR}/temp.dmg"
rm -rf "${TEMP_DIR}"

# Sign the DMG if codesign identity is available
if [ -n "${CODESIGN_ID}" ]; then
    echo "Signing DMG..."
    codesign --force --timestamp --sign "${CODESIGN_ID}" "${DMG_PATH}"
fi

echo "✅ DMG created successfully: ${DMG_PATH}"
echo "   - Contains ${PLUGIN_NAME}.component"
if [ "$ALIAS_CREATED" = true ]; then
    echo "   - Contains alias to Components folder" 
fi
echo "   - Contains installation instructions"