# Techno Machine - Installation & User Guide

## Download

Visit [GitHub Releases](https://github.com/mmmmmmmadman/Techno_Machine/releases) to download the latest version:

- **macOS**: `Techno_Machine_macOS.zip`
- **Windows**: `Techno_Machine_Windows.zip`

---

## macOS Installation

1. Download `Techno_Machine_macOS.zip`
2. Double-click to extract
3. Drag `Techno Machine.app` to your Applications folder
4. On first launch, if you see "cannot be opened" warning:
   - Right-click the app → Select "Open"
   - Or go to System Settings → Privacy & Security → Click "Open Anyway"

---

## Windows Installation

1. Download `Techno_Machine_Windows.zip`
2. Extract to any location
3. Double-click `Techno Machine.exe` to run
4. If Windows Defender warning appears, click "More info" → "Run anyway"

---

## User Interface

### Transport Controls

| Button | Function |
|--------|----------|
| **Play** | Start playback |
| **Stop** | Stop and reset position |
| **Swing** | Cycle swing intensity (Off/1/2/3) |
| **Settings** | Open audio & CV settings |

### Mixer Section

- **Timeline / Foundation / Groove / Lead**: Volume for each drum role
- **Density sliders**: Control rhythm density per role
- **Global**: Overall density offset applied to all roles
- **Fill**: Fill intensity (0-100%)

### DJ Mode

- **Load A / Load B**: Load new rhythm patterns to Deck A or B
- **Crossfader**: Blend between two decks with DJ-style curve
- **Build**: Hold to automate build-up (selectable 4/8/16/32 bars)

---

## Features (v0.3.3)

### Sample Import
- Load WAV/AIFF samples for each voice (8 slots total)
- 2 samples per role: Primary and Secondary
- Samples play alongside synth for layering
- Automatic sample rate conversion
- Sample paths saved between sessions

### Markov Chain Sequencer
- Adds organic variation to rhythm patterns
- Temperature controlled by Density parameter
- Higher density = more random triggers
- Activates when Density > 30%

### Build-up Automation
- Hold the Build button to start automation
- Fill Intensity rises to maximum
- Global Density increases gradually
- Fill Interval shortens progressively
- Release to drop back instantly

### Continuous Fill Mode
- Set Fill Intensity to 100% for continuous fill
- Fill pattern plays indefinitely until intensity is reduced

---

## CV Output Setup

For DC-coupled audio interfaces (e.g., Expert Sleepers ES-8).

1. Click **Settings** button
2. In **Audio Device** tab, select your multi-channel interface
3. Switch to **CV Routing** tab
4. Assign output channels for each CV signal

### CV Signal Types

| Type | Description |
|------|-------------|
| **Trig** | 1ms trigger pulse (0-1V) |
| **Pitch** | Pitch CV, 1V/Oct standard |
| **Vel** | Velocity CV (0-1V) |

### Recommended Channel Layout

- Channels 0-1: Stereo audio output
- Channels 2+: CV outputs

---

## System Requirements

### macOS
- macOS 10.15 (Catalina) or later
- Apple Silicon or Intel processor

### Windows
- Windows 10 or later
- 64-bit processor

### Audio Interface
- Any Core Audio (macOS) or ASIO (Windows) interface
- DC-coupled interface required for CV output

---

## Troubleshooting

### No Sound
1. Open Settings → Audio Device
2. Verify correct output device is selected
3. Check sample rate matches your interface

### No CV Output
1. Confirm your audio interface supports multi-channel output
2. Configure channels in CV Routing tab
3. Channels 0-1 are reserved for audio; CV starts from channel 2

### macOS Cannot Open App
Right-click → Open, or allow in System Settings → Privacy & Security

---

## Contact

- GitHub: https://github.com/mmmmmmmadman/Techno_Machine
- Issues: https://github.com/mmmmmmmadman/Techno_Machine/issues
