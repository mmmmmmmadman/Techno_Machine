# Techno Machine

Standalone Techno music generator with CV output support.

## Documentation

- [English](docs/INSTALL_EN.md) | [繁體中文](docs/INSTALL_ZH.md) | [日本語](docs/INSTALL_JP.md)

## Download

Visit [Releases](https://github.com/mmmmmmmadman/Techno_Machine/releases) to download the latest version.

## Features

- **Drum Synthesis**: 4-role system (Timeline/Foundation/Groove/Lead) with 8 voices
- **DJ Set Mode**: Dual deck architecture with crossfader
- **Markov Chain Sequencer**: Organic rhythm variation controlled by Density
- **Build-up Automation**: Hold-to-build DJ-style tension control
- **Fill System**: Intensity-based fills with continuous mode at 100%
- **CV Output**: 24 CV signals (Trigger/Pitch/Velocity per voice)
- **Multi-channel Audio**: Supports DC-coupled interfaces for CV output

## Requirements

- macOS 10.15+ or Windows 10+
- JUCE 7.x (included as submodule)
- CMake 3.22+
- C++17 compiler

## Build

```bash
git clone --recursive https://github.com/mmmmmmmadman/Techno_Machine.git
cd Techno_Machine
mkdir build && cd build
cmake ..
cmake --build .
```

## CV Output

Channels 0-1 are stereo audio output. Channels 2+ can be routed to CV signals:

| Signal | Description |
|--------|-------------|
| Trigger | 1ms gate pulse |
| Pitch | 1V/Oct (0-1 range) |
| Velocity | 0-1 range |

Use with DC-coupled audio interfaces (e.g., Expert Sleepers ES-8).

## License

MIT License

## Author

MADZINE
