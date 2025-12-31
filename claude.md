# Techno Machine - 獨立應用程式專案規格

**對話模式：** 請使用繁體中文、沒有表情符號的極簡模式，對話不要顯示程式碼內容。

**版本控制：** 此專案僅進行本地 git 管理，不推送到遠端倉庫。

## 專案概述

開發一個跨平台（Windows/macOS）的獨立 Techno 音樂生成器應用程式，具備自動化節奏生成、過門、換段落、換歌功能，內建混音器與效果器。

**開發者背景：** 具有 29 個 VCV Rack 模組開發經驗（MADZINE 品牌），熟悉 C++ 音訊程式設計與模組化合成概念。

---

## 技術架構

### 框架選擇
- **音訊引擎：** JUCE 7.x（跨平台、成熟的音訊框架）
- **語言：** C++17
- **建構系統：** CMake + JUCE CMake API
- **GUI：** JUCE Graphics（可考慮整合 OpenGL 加速）

### 替代方案考量
- RtAudio + Dear ImGui（輕量但需要更多手動整合）
- PortAudio + Qt（較重但 UI 功能豐富）

---

## 核心模組架構

```
TechnoMachine/
├── Source/
│   ├── Core/
│   │   ├── AudioEngine.h/cpp       # 主音訊處理引擎
│   │   ├── Clock.h/cpp             # 主時鐘與同步
│   │   ├── Transport.h/cpp         # 播放控制
│   │   └── MIDIManager.h/cpp       # MIDI I/O
│   │
│   ├── Sequencer/
│   │   ├── EuclideanSequencer.h/cpp    # 歐幾里得節奏生成
│   │   ├── MarkovChain.h/cpp           # 馬可夫鏈序列器
│   │   ├── ConditionalTrigger.h/cpp    # Elektron 式條件觸發
│   │   ├── ParameterLock.h/cpp         # 參數鎖定系統
│   │   └── PatternManager.h/cpp        # Pattern 管理
│   │
│   ├── Arrangement/
│   │   ├── SectionManager.h/cpp        # 段落管理（Intro/Build/Drop/Breakdown）
│   │   ├── TransitionEngine.h/cpp      # 過門自動化
│   │   ├── SongManager.h/cpp           # 歌曲切換邏輯
│   │   └── EnergyController.h/cpp      # 能量曲線控制
│   │
│   ├── Synthesis/
│   │   ├── DrumVoice.h/cpp             # 鼓組合成（Kick/Snare/HiHat/Clap/Perc）
│   │   ├── BassVoice.h/cpp             # Bass 合成
│   │   ├── SynthVoice.h/cpp            # Lead/Pad 合成
│   │   └── SamplePlayer.h/cpp          # 取樣播放器
│   │
│   ├── Effects/
│   │   ├── Filter.h/cpp                # 濾波器（LP/HP/BP）
│   │   ├── Delay.h/cpp                 # 延遲效果
│   │   ├── Reverb.h/cpp                # 混響
│   │   ├── Distortion.h/cpp            # 失真/Saturation
│   │   ├── Compressor.h/cpp            # 壓縮器
│   │   └── Sidechain.h/cpp             # 側鏈壓縮
│   │
│   ├── Mixer/
│   │   ├── Channel.h/cpp               # 單軌道
│   │   ├── MixBus.h/cpp                # 混音匯流排
│   │   ├── MasterBus.h/cpp             # 主輸出
│   │   └── SendReturn.h/cpp            # Send/Return 效果
│   │
│   ├── UI/
│   │   ├── MainWindow.h/cpp
│   │   ├── SequencerView.h/cpp
│   │   ├── MixerView.h/cpp
│   │   ├── ArrangementView.h/cpp
│   │   └── Components/                 # 可重用 UI 元件
│   │
│   └── Utils/
│       ├── DSPUtils.h/cpp              # DSP 工具函數
│       ├── RandomGenerator.h/cpp       # 隨機數生成
│       └── PresetManager.h/cpp         # 預設管理
│
├── Resources/
│   ├── Samples/                        # 預設取樣
│   ├── Presets/                        # 預設檔案
│   └── Skins/                          # UI 主題
│
├── CMakeLists.txt
└── README.md
```

---

## 功能規格

### 1. 節奏生成系統

#### 1.0 多風格節奏系統（來自 UniversalRhythm）

**已實現功能：**
- 4 Role × 2 Voice = 8 聲道架構
- Variation 影響 density、freq/decay
- Ghost Notes（低 velocity 裝飾音）
- Fill 過門系統（每 N bars 自動觸發）
- Interlock 生成（secondary voice 與 primary 互補）

**風格清單（10 種）：**

| 風格 | 適合度 | Techno 化方案 |
|------|--------|---------------|
| Techno | 已實現 | 基礎 |
| Electronic | 高 | 結構相同，調整 hi-hat 密度 |
| Breakbeat | 中高 | syncopated kick 變化 |
| West African | 中 | 採用 swing/hemiola |
| Afro-Cuban | 低 | 只用 tumbao 節奏，不用 Clave |
| Brazilian | 低 | 採用 Tamborim 細碎節奏 |
| Jazz | Techno化 | 保留 ride 節奏型態，straight timing |
| Balkan | Techno化 | 不對稱重音群組映射到 16 步 |
| Indian | Techno化 | Sam/Khali 強空對比 |
| Gamelan | Techno化 | Kotekan 互鎖 + 提高密度 |

#### 1.1 歐幾里得序列器
```cpp
struct EuclideanPattern {
    int steps;      // 1-64
    int pulses;     // 0-steps
    int rotation;   // 0-steps
    float swing;    // 0-100%
    
    std::vector<bool> generate();
};
```

#### 1.2 條件觸發（參考 Elektron）
```cpp
enum class TrigCondition {
    ALWAYS,
    FILL, NOT_FILL,
    PRE, NOT_PRE,
    FIRST,
    RATIO_1_2, RATIO_1_3, RATIO_1_4, // ... X:Y
    PROBABILITY_25, PROBABILITY_50, PROBABILITY_75
};
```

#### 1.3 馬可夫鏈
- 可學習的轉移矩陣
- 分離控制：節奏/音高/力度
- 溫度參數控制隨機性

### 2. 段落與過門系統

#### 2.0 DJ Set 過渡系統（長時間演出用）

**層級結構：**
```
Set（整場演出）
  └── Song（單首歌）
        ├── Style（節奏風格）
        ├── SoundPalette（音色組合）
        └── Sections（段落：Intro/Build/Drop/Breakdown）
```

**過渡機制：**

| 過渡類型 | 說明 | 持續時間 |
|----------|------|----------|
| Style Morph | 節奏權重漸進混合（A→B） | 4-16 bars |
| Sound Crossfade | 音色參數漸變（freq/decay） | 2-8 bars |
| Energy Curve | 密度和強度的整體控制 | 持續 |
| Filter Sweep | 經典 DJ 過渡手法 | 2-4 bars |

**自動換歌觸發條件：**
- 固定時間（每 N 分鐘）
- 能量週期（Drop 後 N bars）
- 手動觸發（系統處理過渡）

**需實作模組：**
- SongManager - 管理歌曲序列和切換
- TransitionEngine - 處理過渡邏輯
- StyleMorpher - 兩種風格間的權重混合

#### 2.1 段落類型
```cpp
enum class SectionType {
    INTRO,          // 16-32 bars, sparse
    BUILDUP,        // 8-16 bars, rising tension
    DROP,           // 16-32 bars, full energy
    BREAKDOWN,      // 16-32 bars, kick out
    OUTRO           // 16-32 bars, fade out
};
```

#### 2.2 自動過門
- **Filter Sweep：** HP/LP 自動化
- **Riser：** 白噪音上升
- **Snare Roll：** 加速滾奏
- **Impact：** Drop 時的衝擊音效
- **Reverse：** 反轉效果

#### 2.3 能量曲線
```cpp
class EnergyController {
    float currentEnergy;  // 0.0 - 1.0
    
    void setTarget(float target, int bars);
    float getEnergy();
    
    // 影響：
    // - 軌道數量
    // - 濾波器開放度
    // - 打擊樂密度
    // - 效果器 wet/dry
};
```

### 3. 混音器規格

#### 3.1 軌道配置
| 軌道 | 類型 | 預設效果 |
|------|------|----------|
| 1-2 | Kick | Compressor, EQ |
| 3-4 | Snare/Clap | Reverb Send, Transient |
| 5-6 | HiHat/Perc | HP Filter, Delay Send |
| 7-8 | Bass | Sidechain, Saturation |
| 9-10 | Synth | Filter, Chorus |
| 11-12 | FX/Riser | Reverb, Delay |

#### 3.2 匯流排架構
- Drum Bus（1-6）
- Bass Bus（7-8）
- Synth Bus（9-10）
- FX Bus（11-12）
- Master Bus

### 4. 效果器規格

#### 4.1 必要效果
- **Filter：** 12/24dB LP/HP/BP, resonance, envelope follower
- **Delay：** Tempo-sync, ping-pong, filter in feedback
- **Reverb：** Plate/Room/Hall, pre-delay, damping
- **Compressor：** Threshold, ratio, attack, release, sidechain input
- **Distortion：** Soft clip, hard clip, bitcrush, wavefold

#### 4.2 Techno 特化效果
- **Pump Compressor：** 自動側鏈壓縮
- **Build Riser：** 自動白噪音/音高上升
- **Stutter：** 節拍同步重複效果
- **Tape Stop：** 減速停止效果

---

## 音訊規格

```cpp
struct AudioConfig {
    int sampleRate = 48000;      // 支援 44100/48000/96000
    int bufferSize = 256;        // 64-2048
    int channels = 2;            // Stereo
    int bitDepth = 32;           // Float
};
```

### DSP 注意事項
- 所有處理使用 float 或 double
- 避免 denormal（加入極小噪音或使用 flush-to-zero）
- 參數變化需要平滑處理（避免 click）
- 濾波器係數需要每 sample 或每 block 更新

---

## MIDI/CV 整合

### MIDI 輸入
- 外部 Clock 同步
- Note/CC 映射到參數
- Program Change 切換預設

### MIDI 輸出
- Clock 輸出
- 每軌道 Note/Gate 輸出
- CC 自動化輸出

### 未來擴展
- OSC 支援
- Ableton Link 同步
- CV 輸出（需要硬體介面）

---

## UI/UX 設計原則

### 基本原則
1. **單視窗操作：** 所有功能在單一視窗內
2. **即時回饋：** 參數變化立即反映
3. **最小點擊：** 常用功能一鍵觸發
4. **深色主題：** 適合低光環境

### 主要視圖
1. **Performance View：** 現場演出用，大按鈕、簡化控制
2. **Edit View：** 詳細參數編輯
3. **Mixer View：** 混音器完整控制
4. **Arrangement View：** 段落時間軸

### 鍵盤快捷鍵
- `Space`: Play/Stop
- `R`: 隨機化當前 Pattern
- `F`: 觸發 Fill
- `1-4`: 切換段落類型
- `M`: 靜音選中軌道
- `S`: Solo 選中軌道

---

## 開發階段

### Phase 1: 核心音訊引擎（2-3 週）
- [ ] JUCE 專案設置
- [ ] 基本音訊 I/O
- [ ] 主時鐘系統
- [ ] 簡單鼓組合成（Kick, HiHat）

### Phase 2: 序列器系統（2-3 週）
- [ ] 歐幾里得序列器
- [ ] Pattern 存儲與切換
- [ ] 條件觸發系統
- [ ] 參數鎖定

### Phase 3: 效果器與混音（2 週）
- [ ] 基本濾波器
- [ ] 延遲與混響
- [ ] 混音器架構
- [ ] 側鏈壓縮

### Phase 4: 段落系統（2 週）
- [ ] 段落定義與切換
- [ ] 自動過門生成
- [ ] 能量曲線控制

### Phase 5: UI 實作（2-3 週）
- [ ] 主視窗框架
- [ ] 序列器視圖
- [ ] 混音器視圖
- [ ] Performance 視圖

### Phase 6: 優化與打包（1-2 週）
- [ ] 效能優化
- [ ] 預設系統
- [ ] Windows/macOS 打包
- [ ] 使用者文件

---

## 參考實作

### 從 VCV Rack 移植的概念
- 歐幾里得節奏生成邏輯
- 模組化信號流設計
- CV 平滑處理技術
- 面板設計美學

### 參考專案
- [JUCE Framework](https://juce.com/)
- [Surge Synthesizer](https://github.com/surge-synthesizer/surge)（開源合成器架構）
- [Vital](https://github.com/mtytel/vital)（現代 UI 設計）

---

## 編碼規範

```cpp
// 命名慣例
class AudioEngine;              // PascalCase for classes
void processBlock();            // camelCase for methods
float sampleRate_;              // trailing underscore for members
const int MAX_VOICES = 16;      // UPPER_CASE for constants

// 文件組織
// .h: 宣告、inline functions
// .cpp: 實作

// 註解
/// @brief 簡短描述
/// @param name 參數說明
/// @return 返回值說明
```

---

## 建構指令

```bash
# 初始化
git clone [repo]
cd TechnoMachine
mkdir build && cd build

# macOS
cmake .. -G Xcode
open TechnoMachine.xcodeproj

# Windows (Visual Studio)
cmake .. -G "Visual Studio 17 2022"
# 開啟 .sln 檔案

# Linux
cmake .. -G Ninja
ninja
```

---

## 注意事項

1. **音訊執行緒安全：** UI 與音訊執行緒分離，使用 lock-free 通訊
2. **記憶體管理：** 音訊執行緒禁止動態分配
3. **延遲補償：** 效果器需報告延遲
4. **CPU 效率：** 使用 SIMD 優化關鍵 DSP

---

## 聯絡與資源

- 研究文件：`techno_music_structure_theory_chinese.md`
- VCV Rack 模組程式碼作為參考
- JUCE 官方文件：https://docs.juce.com/
