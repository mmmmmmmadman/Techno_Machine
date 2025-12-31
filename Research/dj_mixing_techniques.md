# DJ 接歌技術研究：程式化實現方法

## 摘要

本文探討專業 DJ 接歌（mixing/transition）的核心技術，並分析如何在程式生成的節奏系統中實現這些技術。研究涵蓋 beatmatching、phrase matching、EQ mixing、crossfade curves 等關鍵概念，並提出適用於 Techno Machine 的實作方案。

---

## 1. 引言

DJ 接歌的目標是在兩首歌曲之間創造無縫的過渡，維持舞池的能量和節奏連續性。與簡單的「播完換歌」不同，專業 DJ 會使用多種技術讓聽眾幾乎察覺不到歌曲的切換。

對於程式化節奏生成系統，我們面臨一個獨特的挑戰：我們不是混合兩首預錄歌曲，而是要在單一生成器中平滑地從一種節奏模式過渡到另一種。

---

## 2. 核心概念

### 2.1 Beatmatching（節拍對齊）

Beatmatching 是 DJ 混音的基礎，確保兩首歌的節拍完全同步。

**傳統 DJ 方法：**
- 調整 pitch/tempo 使兩首歌的 BPM 一致
- 對齊 downbeat（第一拍）
- 確保 kick drum 同時落點

**程式化實現：**
由於 Techno Machine 使用單一時鐘，BPM 已經同步。關鍵在於確保新舊 pattern 的 downbeat 對齊。

```
實作要點：
- 過渡必須在 bar 邊界開始
- 新 pattern 的第一拍必須對應 bar 的第一拍
- 避免在 bar 中間切換
```

### 2.2 Phrasing（樂句結構）

電子音樂通常遵循固定的樂句結構：

| 單位 | 拍數 | 說明 |
|------|------|------|
| Beat | 1 | 單一節拍 |
| Bar | 4 | 一小節（4/4 拍） |
| Phrase | 32 | 8 bars，標準樂句 |
| Section | 64-128 | 16-32 bars，段落（intro/verse/drop） |

**關鍵原則：**
- 過渡應該在 phrase 邊界發生（每 8 或 16 bars）
- 新歌應該從 phrase 開頭進入
- 避免在樂句中間進行重大改變

### 2.3 歌曲結構（Track Structure）

典型 Techno/House 歌曲結構：

```
Intro (16 bars) → Build-up (8-16 bars) → Drop (16-32 bars)
→ Breakdown (8-16 bars) → Drop (16-32 bars) → Outro (16 bars)
```

**接歌時機：**
- **Intro/Outro mixing**: 最常見，將歌曲 A 的 outro 與歌曲 B 的 intro 重疊
- **Drop to Drop**: 高能量過渡，在兩首歌的 drop 之間切換
- **Breakdown mixing**: 在 breakdown 的低能量段落進行過渡

---

## 3. 過渡技術（Transition Techniques）

### 3.1 Crossfade（交叉淡化）

最基本的過渡方式，漸進調整兩軌音量。

**Crossfade 曲線類型：**

#### Linear Crossfade（線性）
```
Track A: volume = 1 - position
Track B: volume = position
```
- 中間點兩軌各 -6dB
- 適合相似素材，但中間會有音量下降

#### Equal Power Crossfade（等功率）
```
Track A: volume = cos(position * π/2)
Track B: volume = sin(position * π/2)
```
- 中間點兩軌各 -3dB
- 維持感知音量恆定
- **推薦用於 DJ mixing**

#### Logarithmic Crossfade（對數）
```
Track A: volume = 10^(-position * 3) // -30dB range
Track B: volume = 10^(-(1-position) * 3)
```
- 更自然的聽感衰減

**實作參數：**
- 典型 crossfade 時間：4-16 bars
- Techno/House 通常使用較長的 crossfade（8-32 bars）

### 3.2 EQ Mixing（頻率混音）

專業 DJ 不只調整音量，更會操控 EQ 來避免頻率衝突。

#### Bass Swap（低頻切換）

**核心原則：永遠不要同時播放兩個 bassline**

```
過渡時序（以 8 bar 過渡為例）：

Bar 1-2: Track B 進入，但 Bass EQ = 0%
Bar 3-4: Track B 的 Mid/High 逐漸提升
Bar 5-6: 準備 bass swap
Bar 7:   瞬間切換 bass（A→B）
Bar 8:   Track A 淡出
```

**Hard Bass Swap vs Soft Bass Swap：**
- **Hard Swap**: 在一拍內完成 bass 切換，製造衝擊感
- **Soft Swap**: 在 1-2 bars 內漸進交叉 bass

#### 三頻段 EQ 策略

| 頻段 | 頻率範圍 | 過渡策略 |
|------|----------|----------|
| Low (Bass) | 20-200 Hz | 永不重疊，使用 swap |
| Mid | 200-2000 Hz | 可短暫重疊，漸進交叉 |
| High | 2000+ Hz | 可重疊，但注意 harsh 感 |

### 3.3 Filter Sweep（濾波掃頻）

使用 HP/LP filter 創造張力和過渡效果。

#### High-Pass Filter Sweep（常用於 build-up）
```
過渡前：cutoff = 20 Hz（全頻）
過渡中：cutoff 逐漸升高到 500-2000 Hz
高潮點：cutoff 瞬間回到 20 Hz（drop）
```

#### Low-Pass Filter Sweep（常用於 breakdown）
```
過渡前：cutoff = 20000 Hz（全頻）
過渡中：cutoff 逐漸降低到 500-1000 Hz
結束：cutoff 回到 20000 Hz
```

**Filter 曲線：**
- 使用 exponential 曲線比 linear 更自然
- Resonance 可在中間點提升製造「嘯叫」效果

### 3.4 Effect-Based Transitions（效果器過渡）

#### Echo/Delay Out
```
1. 在最後一拍套用 echo 效果
2. 淡出原曲音量
3. echo 尾音與新曲 intro 重疊
```

#### Reverb Wash
```
1. 逐漸增加 reverb wet
2. 在 reverb 最大時切換
3. 新曲進入，逐漸減少 reverb
```

---

## 4. 自動 DJ 系統的學術研究

### 4.1 Beat Tracking 演算法

根據 Len Vande Veire 的研究（2018），自動 DJ 系統使用以下步驟：

1. **Onset Detection**: 分析音訊窗口差異，產生 onset detection curve
2. **Autocorrelation**: 計算自相關函數找出節奏週期
3. **Beat Phase Alignment**: 將 beat 位置對齊 onset curve 的峰值

**準確率：98.2%**（220 首 Drum and Bass 測試）

### 4.2 Downbeat Detection（小節第一拍偵測）

使用機器學習分類器：
- 特徵：能量頻譜、onset 資料
- 分類：判斷每一拍是第 1、2、3、4 拍
- 準確率：98.1%

### 4.3 Structural Segmentation（結構分段）

識別歌曲段落（intro, build-up, drop, breakdown, outro）：
- 建構 self-similarity matrix
- 套用 Foote algorithm 找出邊界
- 規則：結構邊界必須對齊 downbeat

**準確率：94.3%**

### 4.4 Crossfade 實作

學術系統的 crossfade 實作：
- 預處理：time-stretching 對齊 tempo
- EQ filters 套用到音訊片段
- 波形相加：「literally adding (summing) the audio waveforms together」
- 根據 transition type 使用固定的 volume/filter 曲線

---

## 5. Techno Machine 實作方案

### 5.1 系統特性

Techno Machine 的特殊性：
- 單一節奏生成器，非兩首預錄歌曲
- Pattern-based，非連續音訊
- 即時生成，非預處理

### 5.2 建議的過渡架構

#### Phase 1: 準備期（Pre-Transition）
```
觸發條件：距離換歌 N bars 前
動作：
- 記錄當前 pattern 作為「outgoing」
- 生成新 pattern 作為「incoming」
- 標記過渡開始
```

#### Phase 2: 過渡期（Transition）
```
持續時間：4-16 bars（可設定）
每 bar 更新：
- 計算過渡進度 t = current_bar / total_bars
- 套用 crossfade curve
- 套用 EQ mix
- 可選：套用 filter sweep
```

#### Phase 3: 完成期（Post-Transition）
```
動作：
- 釋放 outgoing pattern
- incoming 成為新的 current
- 重置過渡狀態
```

### 5.3 Pattern Crossfade 實作

對於 pattern-based 系統，crossfade 不是音量混合，而是 **onset 機率混合**：

```cpp
// 每個 step 的觸發決策
for (int step = 0; step < 16; step++) {
    float t = transitionProgress;  // 0.0 to 1.0

    // Equal power crossfade
    float outWeight = cos(t * M_PI / 2);
    float inWeight = sin(t * M_PI / 2);

    // Outgoing pattern
    if (outPattern.hasOnset(step)) {
        float prob = outPattern.velocity[step] * outWeight;
        if (random() < prob) {
            triggerVoice(step, prob);
        }
    }

    // Incoming pattern
    if (inPattern.hasOnset(step)) {
        float prob = inPattern.velocity[step] * inWeight;
        if (random() < prob) {
            triggerVoice(step, prob);
        }
    }
}
```

### 5.4 EQ/Filter 實作

#### Per-Role Bass Swap
```cpp
// Foundation role（kick）使用 hard swap
if (role == FOUNDATION) {
    if (t < 0.5) {
        // 只播放 outgoing kick
        playOutgoing = true;
        playIncoming = false;
    } else {
        // 只播放 incoming kick
        playOutgoing = false;
        playIncoming = true;
    }
}
```

#### 其他 Role 使用 Soft Crossfade
```cpp
// Timeline, Groove, Lead 使用漸進混合
float outVol = equalPowerFade(1 - t);
float inVol = equalPowerFade(t);
```

### 5.5 Filter Sweep 整合

```cpp
// HP Filter Sweep during transition
float hpCutoff;
if (t < 0.5) {
    // 上升期：20 Hz → 1000 Hz
    hpCutoff = 20 * pow(50, t * 2);  // exponential
} else {
    // 下降期：1000 Hz → 20 Hz
    hpCutoff = 1000 * pow(0.02, (t - 0.5) * 2);
}
applyHighPassFilter(hpCutoff);
```

### 5.6 Phrase-Aligned Transitions

```cpp
// 確保過渡在 phrase 邊界
void checkTransitionTiming(int currentBar) {
    int barsUntilPhrase = 8 - (currentBar % 8);

    if (shouldStartTransition && barsUntilPhrase <= transitionDuration) {
        // 等待到下一個 phrase 邊界
        if (barsUntilPhrase == transitionDuration) {
            startTransition();
        }
    }
}
```

---

## 6. 建議的 UI 控制

### 6.1 過渡參數
- **Transition Length**: 4/8/16/32 bars
- **Transition Type**: Crossfade / Bass Swap / Filter Sweep / Combined
- **Crossfade Curve**: Linear / Equal Power / Logarithmic

### 6.2 EQ 控制
- **Bass Swap Mode**: Hard / Soft / None
- **Mid/High Blend**: 漸進混合程度

### 6.3 Filter 控制
- **Filter Sweep**: Off / HP Up / LP Down / Both
- **Sweep Range**: 起始/結束 cutoff
- **Resonance**: Filter Q 值

---

## 7. 結論

實現 DJ 風格的程式化接歌需要考慮：

1. **時間對齊**：過渡必須在正確的 phrase 邊界發生
2. **頻率管理**：避免 bass 衝突，使用 swap 或 crossfade
3. **曲線選擇**：Equal power crossfade 最適合音樂混音
4. **效果輔助**：Filter sweep 和 effects 增加過渡的專業感
5. **彈性設定**：不同音樂風格需要不同的過渡策略

對於 Techno Machine，建議優先實作：
1. Equal power crossfade（onset 機率混合）
2. Bass swap for Foundation role
3. HP filter sweep option
4. Phrase-aligned triggering

---

## 參考資料

1. Vande Veire, L. (2018). "From raw audio to a seamless mix: creating an automated DJ system for Drum and Bass." EURASIP Journal on Audio, Speech, and Music Processing.
   - GitHub: github.com/lenvdv/auto-dj

2. DJ.Studio. "16 Basic DJ Transition Techniques Every DJ Should Know"
   - https://dj.studio/blog/basic-transition-techniques

3. Native Instruments. "5 basic DJ transitions to keep your music in the flow"
   - https://blog.native-instruments.com/dj-transitions/

4. Sound On Sound. "Q. Should I use linear or constant-power crossfades?"
   - https://www.soundonsound.com/sound-advice/q-should-use-linear-or-constant-power-crossfades

5. Club Ready DJ School. "Bass Swapping - Don't Make This Common Mistake"
   - https://www.clubreadydjschool.com/tribe-talk/getting-started/bass-swapping-dont-make-this-common-mistake/

6. DJ TechTools. "How to DJ 101: Why You Must Understand Phrasing"
   - https://djtechtools.com/2014/11/16/how-to-dj-101-why-you-must-understand-phrasing/

7. Jupiter Science. "Crossfader Curves: DJ Mixing Formulas & Techniques"
   - https://jupiterscience.com/crossfader-curves-formulas-and-dj-techniques/

---

*文件建立日期：2025-12-31*
*Techno Machine 專案研究文件*
