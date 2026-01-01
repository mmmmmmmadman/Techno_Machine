# Techno Machine - Changelog

## [0.3.3] - 2026-01-01

### New Features
- Sample Import functionality
  - 8 sample slots (2 per role: Primary + Secondary)
  - WAV/AIFF format support with automatic sample rate conversion
  - Samples trigger alongside synth voices for layering
  - Per-role panning: Timeline=L, Foundation/Groove=C, Lead=R
  - Sample paths persist across app restarts

### UI
- Sample panel in bottom-right corner
- Primary/Secondary column layout per role
- Load buttons with filename display

### Technical
- SampleVoice class for single sample playback
- SampleEngine manages 8 sample slots
- Integrated into AudioEngine process chain

---

## [0.3.2] - 2026-01-01

### New Features
- Markov Chain Sequencer integration
  - Per-voice Markov chains with transition matrix
  - Temperature parameter controlled by Density (higher = more random)
  - Additive triggers when Density > 0.3
  - Fill mode boosts Markov trigger probability
- Fill Intensity 100% = Continuous Fill mode
  - Fill stays active indefinitely at max intensity
  - Provides constant fill pattern playback

### Technical
- MarkovChain.hpp with MarkovState (REST/HIT) and transition logic
- MarkovEngine manages 8 voice chains with role-specific base probabilities
- Integrated into getMixDecision for organic rhythm variation

---

## [0.3.1] - 2026-01-01

### New Features
- Build-up button for DJ-style tension control
  - Hold button to start build-up automation
  - Selectable duration (4/8/16/32 bars)
  - Fill Intensity: linear rise to maximum (1.0)
  - Global Density: linear rise to maximum (0.5)
  - Fill Interval: progressive shortening (original → 2 bars → 1 bar)
  - Smooth progress with sub-bar precision (sixteenth-based)
  - Release button to restore original values (instant drop effect)

### UI
- Build button with bar selector next to crossfader
- Visual feedback: button lights up during build-up, shows percentage

### Bug Fixes
- Fixed default Swing level 1 not showing at startup (crossfader init issue)
- Fixed parameter jitter during build-up (use direct value assignment)

---

## [0.3.0] - 2026-01-01

### New Features
- Enhanced Fill System with intensity-based control
  - Fill Intensity slider (0-100%) controls all fill parameters
  - Length: 2-14 steps based on intensity
  - Complexity: 1-4 roles participate (Timeline → Foundation → Groove → Lead)
  - Density: 0.4-0.9 fill pattern density
  - Accent probability: High intensity adds accent hits
- Swing from style profile (auto-syncs when crossfader moves or deck loads)

### UI
- Fill Intensity slider next to Global Density

### Technical
- FillSettings struct with intensity-derived parameters
- TechnoPatternEngine integrates FillSettings for fill generation

---

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
- [x] Swing from style profile
- [x] Markov chain sequencer
- [x] Enhanced fill system (length/complexity/density control)
- [x] Sample import (WAV/AIFF per voice)
- [ ] Conditional triggers (Elektron-style)
- [ ] Parameter locking
- [ ] Section management (Intro/Build/Drop/Breakdown)
- [ ] Transition engine
- [ ] Effects (Filter, Delay, Reverb)
- [ ] Mixer with send/return
- [ ] MIDI sync
- [ ] Preset system
