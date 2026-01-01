# Techno Machine - Changelog

## [0.2.0] - 2025-01-01

### New Features
- CV output routing (24 signals: 8 voices × Trigger/Pitch/Velocity)
- Audio device selection via Settings window
- Settings persistence (CV routing and audio device saved on exit)

### UI
- Settings window with tabbed interface (Audio Device / CV Routing)
- Settings button in main window toolbar
- Compact CV routing panel organized by Role

### Technical
- Migrated from AudioAppComponent to AudioIODeviceCallback for multi-channel support
- CVOutputRouter class with 1V/Oct pitch CV conversion
- ApplicationProperties for settings persistence

---

## [0.1.0] - 2024-12-31

### Core Features
- JUCE 7.x standalone application framework
- Audio engine with stereo output
- Transport system (Play/Stop/Reset)
- Tempo control (80-180 BPM)
- Swing control (Off/1/2/3 levels)

### Drum Synthesis
- 4-role drum system: Timeline (Hi-Hat), Foundation (Kick), Groove (Clap), Lead (Rim)
- Per-role level control
- Per-role density control
- Global density offset

### DJ Set System
- Dual deck architecture (Deck A / Deck B)
- Crossfader with smooth transition
- Load A/B buttons for switching patterns
- Role style display showing active deck patterns

### UI
- Dark warm theme with vibrant pink accent (#ff9eb0)
- Cross-platform embedded font (Inter Thin)
- Fixed window size (1000x420)
- 4 level faders + 4 density faders layout
- Real-time bar/beat/sixteenth display

### Technical
- C++17 standard
- CMake build system
- Embedded font via BinaryData
- Custom LookAndFeel for consistent typography

---

## Development Notes

### Architecture
```
TechnoMachine/
├── Source/
│   ├── Core/           # AudioEngine, Clock, Transport
│   ├── Sequencer/      # DrumSequencer, PatternManager
│   ├── Synthesis/      # DrumVoice
│   ├── Arrangement/    # SongManager (planned)
│   └── UI/             # (reserved for future components)
├── Resources/
│   └── Fonts/          # Inter-Thin.ttf
└── Research/           # Design documents
```

### Role Mapping
| Role | Sound | Default Level |
|------|-------|---------------|
| Timeline | Hi-Hat | 0.5 |
| Foundation | Kick | 1.0 |
| Groove | Clap | 0.7 |
| Lead | Rim | 0.5 |

### Color Palette
- Background: #0e0c0c (dark warm black)
- Panel: #201a1a (dark warm gray)
- Accent: #ff9eb0 (vibrant pink)
- Accent Dim: #c08090 (muted pink)
- Text: #ffffff / #c8b8b8

---

## Planned Features

- [x] CV output routing
- [x] Audio device selection
- [ ] Euclidean sequencer
- [ ] Conditional triggers (Elektron-style)
- [ ] Section management (Intro/Build/Drop/Breakdown)
- [ ] Transition engine
- [ ] Effects (Filter, Delay, Reverb)
- [ ] Mixer with send/return
- [ ] MIDI sync
- [ ] Preset system
