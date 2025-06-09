# AUSoundTouch - High-Quality Pitch & Tempo Control for macOS

AUSoundTouch is a professional audio plugin for macOS that lets you change the pitch, tempo, and speed of any audio in real-time with exceptional quality. Unlike the basic pitch shifter that comes with macOS, AUSoundTouch uses advanced algorithms to maintain crystal-clear audio quality even with extreme pitch changes.

## What Can It Do?

- **🎵 Pitch Shifting**: Change pitch up to ±40 semitones without the "chipmunk" effect
- **⏱️ Tempo Control**: Speed up or slow down audio from 10% to 1000% of original speed
- **🎚️ Speed Control**: Change playback speed while maintaining (or changing) pitch
- **🎛️ Professional Quality**: Uses the mature SoundTouch audio processing library
- **🔌 Works Everywhere**: Compatible with Logic Pro, GarageBand, Reaper, and any Audio Unit v3 host

## Installation

1. **Download** the latest `AUSoundTouch.dmg` from the [Releases](https://github.com/allquixotic/ausoundtouch/releases) page

2. **Open** the downloaded DMG file

3. **Drag** the `AUSoundTouch.component` file to the Audio Unit Components folder shortcut shown in the DMG

   - Or manually copy to: `~/Library/Audio/Plug-Ins/Components/`

4. **Restart** your audio software (DAW)

5. **Find** AUSoundTouch in your plugin list - it will appear under Audio Units as:
   - **Manufacturer**: SnMc
   - **Effect**: AUSoundTouch

## How to Use

Once installed, AUSoundTouch appears as an audio effect in your DAW:

1. Add it to any audio track
2. Use the three main controls:
   - **Pitch**: Shift pitch up or down in semitones
   - **Tempo**: Change speed without affecting pitch
   - **Speed**: Change overall playback rate

Each control features:
- A horizontal slider for smooth adjustments
- An editable text box for precise numeric input (just type the number, no units needed)
- A reset button to return to default
- Full automation support for all parameters

**Buffering Options** (below Speed control):
- **Minimal**: Lowest latency mode for live performance
- **Normal**: Balanced setting for most uses (default)
- **Extra**: Maximum stability for complex processing

## Why AUSoundTouch?

macOS includes a basic pitch shifter (AUPitch), but it has limitations:
- Poor quality at extreme pitch shifts
- Limited range
- Introduces artifacts and distortion

AUSoundTouch provides:
- ✨ Superior sound quality using SoundTouch algorithms
- 🎯 Much wider pitch range (±40 semitones vs ±12)
- 🎵 Natural-sounding results even at extreme settings
- 🎚️ Independent tempo and pitch control

## System Requirements

- macOS Sonoma (14.0) or later
- Any Audio Unit compatible host application
- Apple Silicon (M1/M2/M3) or Intel Mac

## About

AUSoundTouch is a modern, native macOS implementation inspired by the classic rbpitch plugin for Rhythmbox. It brings the same high-quality pitch shifting technology to macOS as a proper Audio Unit v3 plugin, fully compatible with modern DAWs and signed/notarized for security.

## License

This project is licensed under the MIT License. The SoundTouch library is used under the LGPL license.

## Troubleshooting

### Plugin doesn't appear in my DAW
1. Make sure you copied it to the correct folder: `~/Library/Audio/Plug-Ins/Components/`
2. Restart your DAW
3. Look under Audio Units → YRCM → AUSoundTouch

### Changes not taking effect after updating
macOS caches Audio Units aggressively. To ensure you're using the latest version:
1. Quit your DAW
2. Remove the old plugin: `rm -rf ~/Library/Audio/Plug-Ins/Components/AUSoundTouch.component`
3. Clear the cache: `killall -9 AudioComponentRegistrar`
4. Install the new version
5. Restart your DAW

### Audio dropouts or glitches
- Try changing the Buffering option:
  - Use "Extra" for more stability if you hear dropouts
  - Use "Minimal" if you need lower latency for live performance
- Increase your DAW's buffer size for better performance
- The plugin may briefly pass through dry signal during initial buffering - this is normal

## Support

For issues, feature requests, or questions, please visit our [GitHub Issues](https://github.com/allquixotic/ausoundtouch/issues) page.

---

<div align="center">

🎵 ✨ **Proudly vibe coded using Claude Code with Anthropic Claude 4** ✨ 🎵

</div>