# AUSoundTouch Build Runner
# Simple commands for common tasks
#
# IMPORTANT NOTE ABOUT PLUGIN INSTALLATION:
# macOS aggressively caches Audio Units. To ensure you're always testing the
# latest build, both 'make install' and 'make install-release' now:
#   1. Remove any existing plugin first
#   2. Install the new version
#   3. Clear the Audio Unit cache (kills AudioComponentRegistrar)
# This prevents the common issue where changes don't appear to take effect.
#
# For release builds, the recommended workflow is:
#   make clean && make release && make install-release && make func-release
# Or simply use the shortcuts:
#   make c && make release && make i-release && make func-release

# Directories - note that we work from the AUSoundTouch subdirectory
PROJECT_DIR = AUSoundTouch
BUILD_DIR = $(PROJECT_DIR)/build
PLUGIN_NAME = AUSoundTouch
COMPONENT_PATH = ~/Library/Audio/Plug-Ins/Components/$(PLUGIN_NAME).component

# Build configuration
USE_SYSTEM_SOUNDTOUCH ?= OFF
CMAKE_EXTRA_ARGS ?=

# SoundTouch PKG_CONFIG path (only needed if USE_SYSTEM_SOUNDTOUCH=ON)
ifeq ($(USE_SYSTEM_SOUNDTOUCH),ON)
	PKG_CONFIG_PATH := /opt/homebrew/Cellar/sound-touch/2.4.0/lib/pkgconfig
	CMAKE_CONFIG_PREFIX := PKG_CONFIG_PATH=$(PKG_CONFIG_PATH)
endif

# Help (default target)
.PHONY: help
help:
	@echo "AUSoundTouch Build Commands:"
	@echo ""
	@echo "Debug Commands:"
	@echo "  make build       - Build debug version (default)"
	@echo "  make test        - Run all tests (debug)"
	@echo "  make unit        - Run unit tests only (debug)"
	@echo "  make func        - Run functional tests only (debug)"
	@echo "  make pitch       - Run pitch shift validation test (debug)"
	@echo "  make pitch-play  - Run pitch validation with audio playback (debug)"
	@echo "  make install     - Install debug plugin"
	@echo "  make reinstall   - Remove and reinstall debug plugin"
	@echo "  make leaks       - Check for memory leaks (debug)"
	@echo ""
	@echo "Release Commands:"
	@echo "  make release     - Build release version"
	@echo "  make test-release- Run all tests (release)"
	@echo "  make unit-release- Run unit tests only (release)"
	@echo "  make func-release- Run functional tests only (release)"
	@echo "  make pitch-release - Run pitch shift validation test (release)"
	@echo "  make pitch-play-release - Run pitch validation with audio playback (release)"
	@echo "  make install-release - Install release plugin (with signing)"
	@echo "  make reinstall-release - Remove and reinstall release plugin"
	@echo "  make leaks-release - Check for memory leaks (release)"
	@echo ""
	@echo "Distribution Commands:"
	@echo "  make sign        - Sign the release build"
	@echo "  make notarize    - Full notarization (sign, submit, staple)"
	@echo "  make dmg         - Create distributable DMG"
	@echo "  make dist        - Complete distribution workflow"
	@echo ""
	@echo "Other Commands:"
	@echo "  make deps        - Check build dependencies"
	@echo "  make clean       - Clean build directory"
	@echo "  make remove      - Remove installed plugin"
	@echo "  make validate    - Run auval validation"
	@echo "  make all         - Clean, build, test, and install (debug)"
	@echo "  make quick       - Quick build and unit test (debug)"
	@echo "  make quick-release - Clean, build, install, and test (release)"
	@echo ""
	@echo "Ultra-Short Aliases:"
	@echo "  make b           - build"
	@echo "  make t           - test"
	@echo "  make i           - install"
	@echo "  make ir          - install-release"
	@echo "  make r           - reinstall"
	@echo "  make rr          - reinstall-release"
	@echo "  make v           - validate"
	@echo "  make c           - clean"
	@echo "  make p           - pitch"
	@echo "  make pp          - pitch-play"
	@echo "  make qr          - quick-release"

# Check dependencies
.PHONY: deps
deps:
	@./check-dependencies.sh

# Build debug (default)
.PHONY: build
build:
	@echo "Building Debug..."
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && $(CMAKE_CONFIG_PREFIX) cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug -DUSE_SYSTEM_SOUNDTOUCH=$(USE_SYSTEM_SOUNDTOUCH) $(CMAKE_EXTRA_ARGS) ..
	@cd $(BUILD_DIR) && ninja

# Build release
.PHONY: release
release:
	@echo "Building Release..."
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && $(CMAKE_CONFIG_PREFIX) cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DUSE_SYSTEM_SOUNDTOUCH=$(USE_SYSTEM_SOUNDTOUCH) $(CMAKE_EXTRA_ARGS) ..
	@cd $(BUILD_DIR) && ninja

# Clean
.PHONY: clean
clean:
	@echo "Cleaning build directory..."
	@rm -rf $(BUILD_DIR)

# Run all tests (debug)
.PHONY: test
test:
	@echo "Running all tests (Debug)..."
	@cd $(BUILD_DIR) && ctest --output-on-failure -C Debug

# Run all tests (release)
.PHONY: test-release
test-release:
	@echo "Running all tests (Release)..."
	@cd $(BUILD_DIR) && ctest --output-on-failure -C Release

# Run unit tests only (debug)
.PHONY: unit
unit:
	@echo "Running unit tests (Debug)..."
	@if [ -f "$(BUILD_DIR)/AUSoundTouchTests_artefacts/Debug/AUSoundTouchTests" ]; then \
		$(BUILD_DIR)/AUSoundTouchTests_artefacts/Debug/AUSoundTouchTests -s; \
	else \
		echo "Debug unit tests not built. Run 'make build' first."; \
	fi

# Run unit tests only (release)
.PHONY: unit-release
unit-release:
	@echo "Running unit tests (Release)..."
	@if [ -f "$(BUILD_DIR)/AUSoundTouchTests_artefacts/Release/AUSoundTouchTests" ]; then \
		$(BUILD_DIR)/AUSoundTouchTests_artefacts/Release/AUSoundTouchTests -s; \
	else \
		echo "Release unit tests not built. Run 'make release' first."; \
	fi

# Run functional tests only (debug)
.PHONY: func
func:
	@echo "Running functional tests (Debug)..."
	@if [ -f "$(BUILD_DIR)/AUSoundTouchFunctionalTests_artefacts/Debug/AUSoundTouchFunctionalTests" ]; then \
		$(BUILD_DIR)/AUSoundTouchFunctionalTests_artefacts/Debug/AUSoundTouchFunctionalTests; \
	else \
		echo "Debug functional tests not built. Run 'make build' first."; \
	fi

# Run functional tests only (release)
.PHONY: func-release
func-release:
	@echo "Running functional tests (Release)..."
	@if [ -f "$(BUILD_DIR)/AUSoundTouchFunctionalTests_artefacts/Release/AUSoundTouchFunctionalTests" ]; then \
		$(BUILD_DIR)/AUSoundTouchFunctionalTests_artefacts/Release/AUSoundTouchFunctionalTests; \
	else \
		echo "Release functional tests not built. Run 'make release' first."; \
	fi

# Run pitch shift validation test (debug)
.PHONY: pitch
pitch:
	@echo "Running pitch shift validation test (Debug)..."
	@if [ -f "$(BUILD_DIR)/PitchShiftValidationTest_artefacts/Debug/PitchShiftValidationTest" ]; then \
		$(BUILD_DIR)/PitchShiftValidationTest_artefacts/Debug/PitchShiftValidationTest; \
	else \
		echo "Debug pitch validation test not built. Run 'make build' first."; \
	fi

# Run pitch shift validation test with audio playback (debug)
.PHONY: pitch-play
pitch-play:
	@echo "Running pitch shift validation test with audio playback (Debug)..."
	@if [ -f "$(BUILD_DIR)/PitchShiftValidationTest_artefacts/Debug/PitchShiftValidationTest" ]; then \
		$(BUILD_DIR)/PitchShiftValidationTest_artefacts/Debug/PitchShiftValidationTest --play; \
	else \
		echo "Debug pitch validation test not built. Run 'make build' first."; \
	fi

# Run pitch shift validation test (release)
.PHONY: pitch-release
pitch-release:
	@echo "Running pitch shift validation test (Release)..."
	@if [ -f "$(BUILD_DIR)/PitchShiftValidationTest_artefacts/Release/PitchShiftValidationTest" ]; then \
		$(BUILD_DIR)/PitchShiftValidationTest_artefacts/Release/PitchShiftValidationTest; \
	else \
		echo "Release pitch validation test not built. Run 'make release' first."; \
	fi

# Run pitch shift validation test with audio playback (release)
.PHONY: pitch-play-release
pitch-play-release:
	@echo "Running pitch shift validation test with audio playback (Release)..."
	@if [ -f "$(BUILD_DIR)/PitchShiftValidationTest_artefacts/Release/PitchShiftValidationTest" ]; then \
		$(BUILD_DIR)/PitchShiftValidationTest_artefacts/Release/PitchShiftValidationTest --play; \
	else \
		echo "Release pitch validation test not built. Run 'make release' first."; \
	fi

# Install debug plugin
# IMPORTANT: Always removes existing plugin first to avoid macOS AU cache issues
.PHONY: install
install:
	@echo "Installing Debug plugin..."
	@if [ -d "$(BUILD_DIR)/$(PLUGIN_NAME)_artefacts/Debug/AU/$(PLUGIN_NAME).component" ]; then \
		echo "Removing existing plugin to avoid cache issues..."; \
		rm -rf $(COMPONENT_PATH); \
		cp -R $(BUILD_DIR)/$(PLUGIN_NAME)_artefacts/Debug/AU/$(PLUGIN_NAME).component $(COMPONENT_PATH); \
		echo "Installed Debug build to $(COMPONENT_PATH)"; \
		echo "Clearing Audio Unit cache..."; \
		killall -9 AudioComponentRegistrar 2>/dev/null || true; \
		echo "Installation complete. You may need to restart your DAW."; \
	else \
		echo "Debug plugin not built. Run 'make build' first."; \
	fi

# Install release plugin (with signing)
# IMPORTANT: Always removes existing plugin first to avoid macOS AU cache issues
.PHONY: install-release
install-release:
	@echo "Installing Release plugin..."
	@if [ -d "$(BUILD_DIR)/$(PLUGIN_NAME)_artefacts/Release/AU/$(PLUGIN_NAME).component" ]; then \
		echo "Removing existing plugin to avoid cache issues..."; \
		rm -rf $(COMPONENT_PATH); \
		echo "Signing Release build before installation..."; \
		dotenvx run -- sh -c 'codesign --deep --force --options runtime \
			--timestamp --sign "$$CODESIGN_ID" \
			$(BUILD_DIR)/$(PLUGIN_NAME)_artefacts/Release/AU/$(PLUGIN_NAME).component' || \
			{ echo "Warning: Code signing failed, continuing with installation"; }; \
		cp -R $(BUILD_DIR)/$(PLUGIN_NAME)_artefacts/Release/AU/$(PLUGIN_NAME).component $(COMPONENT_PATH); \
		echo "Installed Release build to $(COMPONENT_PATH)"; \
		echo "Clearing Audio Unit cache..."; \
		killall -9 AudioComponentRegistrar 2>/dev/null || true; \
		echo "Installation complete. You may need to restart your DAW."; \
	else \
		echo "Release plugin not built. Run 'make release' first."; \
	fi

# Remove plugin
.PHONY: remove
remove:
	@echo "Removing installed plugin..."
	@rm -rf $(COMPONENT_PATH)
	@echo "Removed $(COMPONENT_PATH)"

# Reinstall debug plugin
.PHONY: reinstall
reinstall: remove install

# Reinstall release plugin
.PHONY: reinstall-release
reinstall-release: remove install-release

# Validate with auval
.PHONY: validate
validate:
	@echo "Validating plugin with auval..."
	@auval -v aufx ASTc YRCM

# Setup notarization credentials
.PHONY: setup-notary
setup-notary:
	@echo "Setting up notarization credentials..."
	@dotenvx run -- sh -c 'if [ -z "$$APPLE_EMAIL" ] || [ -z "$$APP_PASSWORD" ] || [ -z "$$CODESIGN_ID" ]; then \
		echo "Error: Missing required environment variables."; \
		echo "Please ensure APPLE_EMAIL, APP_PASSWORD, and CODESIGN_ID are set in .env"; \
		exit 1; \
	fi; \
	TEAM_ID=$$(echo "$$CODESIGN_ID" | grep -o "([^)]*)" | tr -d "()"); \
	echo "Using Team ID: $$TEAM_ID"; \
	echo "Using Apple ID: $$APPLE_EMAIL"; \
	echo "Using Profile: $$NOTARY_PROFILE"; \
	xcrun notarytool store-credentials "$$NOTARY_PROFILE" \
		--apple-id "$$APPLE_EMAIL" \
		--team-id "$$TEAM_ID" \
		--password "$$APP_PASSWORD"; \
	echo "✅ Notarization credentials stored successfully"'

# Sign release build
.PHONY: sign
sign:
	@echo "Signing release build..."
	@if [ -d "$(BUILD_DIR)/$(PLUGIN_NAME)_artefacts/Release/AU/$(PLUGIN_NAME).component" ]; then \
		dotenvx run -- sh -c 'codesign --deep --force --options runtime \
			--timestamp --sign "$$CODESIGN_ID" \
			$(BUILD_DIR)/$(PLUGIN_NAME)_artefacts/Release/AU/$(PLUGIN_NAME).component'; \
		echo "Signed successfully"; \
	else \
		echo "Release build not found. Run 'make release' first."; \
	fi

# Full notarization process (sign, notarize, staple)
.PHONY: notarize
notarize:
	@echo "Starting full notarization process..."
	@if [ ! -d "$(BUILD_DIR)/$(PLUGIN_NAME)_artefacts/Release/AU/$(PLUGIN_NAME).component" ]; then \
		echo "Release build not found. Run 'make release' first."; \
		exit 1; \
	fi
	@echo "Step 1/5: Signing component..."
	@dotenvx run -- sh -c 'codesign --deep --force --options runtime \
		--timestamp --sign "$$CODESIGN_ID" \
		$(BUILD_DIR)/$(PLUGIN_NAME)_artefacts/Release/AU/$(PLUGIN_NAME).component'
	@echo "Step 2/5: Creating ZIP for notarization..."
	@cd $(BUILD_DIR)/$(PLUGIN_NAME)_artefacts/Release/AU && \
		zip -r $(PLUGIN_NAME).zip $(PLUGIN_NAME).component
	@echo "Step 3/5: Submitting to Apple for notarization..."
	@dotenvx run -- sh -c 'xcrun notarytool submit \
		$(BUILD_DIR)/$(PLUGIN_NAME)_artefacts/Release/AU/$(PLUGIN_NAME).zip \
		--keychain-profile "$$NOTARY_PROFILE" \
		--wait || (echo "❌ Notarization failed. Try running '\''make setup-notary'\'' to update credentials."; exit 1)'
	@echo "Step 4/5: Stapling ticket to component..."
	@xcrun stapler staple $(BUILD_DIR)/$(PLUGIN_NAME)_artefacts/Release/AU/$(PLUGIN_NAME).component
	@echo "Step 5/5: Verifying notarization..."
	@xcrun stapler validate $(BUILD_DIR)/$(PLUGIN_NAME)_artefacts/Release/AU/$(PLUGIN_NAME).component
	@spctl --assess --type execute -vvvv $(BUILD_DIR)/$(PLUGIN_NAME)_artefacts/Release/AU/$(PLUGIN_NAME).component || true
	@echo "✅ Notarization complete! Component is ready for distribution."
	@echo "   Notarized component: $(BUILD_DIR)/$(PLUGIN_NAME)_artefacts/Release/AU/$(PLUGIN_NAME).component"

# Create a distributable DMG
.PHONY: dmg
dmg:
	@echo "Creating distributable DMG..."
	@if [ ! -d "$(BUILD_DIR)/$(PLUGIN_NAME)_artefacts/Release/AU/$(PLUGIN_NAME).component" ]; then \
		echo "Release build not found. Run 'make release' first."; \
		exit 1; \
	fi
	@if dotenvx run -- sh -c '[ -n "$$CODESIGN_ID" ]'; then \
		dotenvx run -- ./create-dmg.sh; \
	else \
		./create-dmg.sh; \
	fi

# Complete distribution workflow: build, test, sign, notarize, create DMG
.PHONY: dist
dist: release test-release
	@echo "▶︎ Starting distribution build process..."
	@echo "Step 1/6: Release build complete"
	@echo "Step 2/6: Tests passed"
	@echo "Step 3/6: Signing component..."
	@dotenvx run -- sh -c 'codesign --deep --force --options runtime \
		--timestamp --sign "$$CODESIGN_ID" \
		$(BUILD_DIR)/$(PLUGIN_NAME)_artefacts/Release/AU/$(PLUGIN_NAME).component'
	@echo "Step 4/6: Creating ZIP for notarization..."
	@cd $(BUILD_DIR)/$(PLUGIN_NAME)_artefacts/Release/AU && \
		zip -r $(PLUGIN_NAME).zip $(PLUGIN_NAME).component
	@echo "Step 5/6: Submitting to Apple for notarization..."
	@dotenvx run -- sh -c 'xcrun notarytool submit \
		$(BUILD_DIR)/$(PLUGIN_NAME)_artefacts/Release/AU/$(PLUGIN_NAME).zip \
		--keychain-profile "$$NOTARY_PROFILE" \
		--wait || (echo "❌ Notarization failed. Try running '\''make setup-notary'\'' to update credentials."; exit 1)'
	@echo "Step 6/6: Stapling ticket..."
	@xcrun stapler staple $(BUILD_DIR)/$(PLUGIN_NAME)_artefacts/Release/AU/$(PLUGIN_NAME).component
	@xcrun stapler validate $(BUILD_DIR)/$(PLUGIN_NAME)_artefacts/Release/AU/$(PLUGIN_NAME).component
	@echo "▶︎ Creating DMG with Components folder shortcut..."
	@dotenvx run -- ./create-dmg.sh
	@echo "▶︎ Submitting DMG for notarization..."
	@dotenvx run -- sh -c 'xcrun notarytool submit $(BUILD_DIR)/$(PLUGIN_NAME).dmg \
		--keychain-profile "$$NOTARY_PROFILE" \
		--wait'
	@echo "▶︎ Stapling ticket to DMG..."
	@xcrun stapler staple $(BUILD_DIR)/$(PLUGIN_NAME).dmg
	@xcrun stapler validate $(BUILD_DIR)/$(PLUGIN_NAME).dmg
	@echo "▶︎ Final assessment of .component..."
	@spctl --assess --type execute -vvvv $(BUILD_DIR)/$(PLUGIN_NAME)_artefacts/Release/AU/$(PLUGIN_NAME).component || true
	@echo "▶︎ Final assessment of DMG..."
	@spctl --assess --type open --context context:primary-signature -vvvv $(BUILD_DIR)/$(PLUGIN_NAME).dmg || true
	@echo "🎉 Distribution package ready: $(BUILD_DIR)/$(PLUGIN_NAME).dmg"

# Full workflow: clean, build, test, install
.PHONY: all
all: clean build test install
	@echo "Complete build cycle finished!"

# Quick rebuild and test
.PHONY: quick
quick: build unit
	@echo "Quick build and test complete!"

# Quick release workflow (clean, build, install, test)
.PHONY: quick-release
quick-release: clean release install-release func-release
	@echo "Quick release workflow complete!"

# Memory leak check (debug)
.PHONY: leaks
leaks:
	@echo "Checking for memory leaks (Debug)..."
	@if [ -f "$(BUILD_DIR)/AUSoundTouchTests_artefacts/Debug/AUSoundTouchTests" ]; then \
		leaks --atExit -- $(BUILD_DIR)/AUSoundTouchTests_artefacts/Debug/AUSoundTouchTests; \
	else \
		echo "Debug unit tests not built. Run 'make build' first."; \
	fi

# Memory leak check (release)
.PHONY: leaks-release
leaks-release:
	@echo "Checking for memory leaks (Release)..."
	@if [ -f "$(BUILD_DIR)/AUSoundTouchTests_artefacts/Release/AUSoundTouchTests" ]; then \
		leaks --atExit -- $(BUILD_DIR)/AUSoundTouchTests_artefacts/Release/AUSoundTouchTests; \
	else \
		echo "Release unit tests not built. Run 'make release' first."; \
	fi

# Development shortcuts
.PHONY: b t i ir r rr v c p pp qr
b: build
t: test  
i: install
ir: install-release
r: reinstall
rr: reinstall-release
v: validate
c: clean
p: pitch
pp: pitch-play
qr: quick-release

.DEFAULT_GOAL := help