/**
 * TechnoPattern.h
 * Techno Machine - 8 聲道節奏生成系統
 *
 * 完全套用 UniversalRhythm 設計：
 * - 4 Role × 2 Voice = 8 聲道
 * - Primary voice: 主要節奏
 * - Secondary voice: 使用 Interlock 生成的補充節奏
 * - Variation 影響 density 和隨機性
 * - 支援 10 種風格切換
 */

#pragma once

#include <vector>
#include <random>
#include <algorithm>
#include <cmath>
#include "StyleProfiles.hpp"
#include "../Synthesis/MinimalDrumSynth.h"

namespace TechnoMachine {

/**
 * Pattern 結構
 */
struct Pattern {
    std::vector<float> velocities;  // 0.0 = 無觸發, 0.01-1.0 = velocity
    int length;

    Pattern(int len = 16) :
        velocities(static_cast<size_t>(std::max(1, len)), 0.0f),
        length(std::max(1, len)) {}

    void clear() {
        std::fill(velocities.begin(), velocities.end(), 0.0f);
    }

    bool hasOnset(int pos) const {
        if (length <= 0) return false;
        return velocities[static_cast<size_t>(pos % length)] > 0.0f;
    }

    float getVelocity(int pos) const {
        if (length <= 0) return 0.0f;
        return velocities[static_cast<size_t>(pos % length)];
    }

    void setOnset(int pos, float velocity = 0.7f) {
        if (length <= 0) return;
        velocities[static_cast<size_t>(pos % length)] = std::clamp(velocity, 0.01f, 1.0f);
    }

    void clearOnset(int pos) {
        if (length <= 0) return;
        velocities[static_cast<size_t>(pos % length)] = 0.0f;
    }
};

/**
 * 風格權重存取器
 * 支援 per-role 不同風格（複合風格）
 */
class StyleWeights {
public:
    // 設定統一風格（所有角色使用相同風格）
    static void setStyle(const StyleProfile* style) {
        for (int i = 0; i < NUM_ROLES; i++) {
            roleStyles_[i] = style;
        }
    }

    // 設定統一風格（by index）
    static void setStyle(int styleIdx) {
        if (styleIdx >= 0 && styleIdx < NUM_STYLES) {
            setStyle(STYLES[styleIdx]);
        }
    }

    // 設定 per-role 風格（複合風格）
    static void setCompositeStyle(const int* roleStyleIndices) {
        for (int i = 0; i < NUM_ROLES; i++) {
            int idx = roleStyleIndices[i];
            if (idx >= 0 && idx < NUM_STYLES) {
                roleStyles_[i] = STYLES[idx];
            }
        }
    }

    // 設定單一角色的風格
    static void setRoleStyle(Role role, int styleIdx) {
        if (role >= 0 && role < NUM_ROLES && styleIdx >= 0 && styleIdx < NUM_STYLES) {
            roleStyles_[role] = STYLES[styleIdx];
        }
    }

    static const StyleProfile* getStyle(Role role) {
        if (role >= 0 && role < NUM_ROLES && roleStyles_[role]) {
            return roleStyles_[role];
        }
        return &STYLE_TECHNO;
    }

    // 舊介面相容（回傳第一個角色的風格）
    static const StyleProfile* getStyle() {
        return roleStyles_[0] ? roleStyles_[0] : &STYLE_TECHNO;
    }

    static const float* getWeights(Role role) {
        const StyleProfile* s = getStyle(role);
        switch (role) {
            case TIMELINE: return s->timeline;
            case FOUNDATION: return s->foundation;
            case GROOVE: return s->groove;
            case LEAD: return s->lead;
            default: return s->timeline;
        }
    }

    static float getDensityMin(Role role) {
        return getStyle(role)->densityRange[role][0];
    }

    static float getDensityMax(Role role) {
        return getStyle(role)->densityRange[role][1];
    }

private:
    static inline const StyleProfile* roleStyles_[NUM_ROLES] = {
        &STYLE_TECHNO, &STYLE_TECHNO, &STYLE_TECHNO, &STYLE_TECHNO
    };
};

/**
 * 8 聲道 Pattern 組合（4 Role × 2 Voice）
 */
struct MultiVoicePatterns {
    Pattern patterns[NUM_VOICES];

    MultiVoicePatterns(int length = 16) {
        for (int i = 0; i < NUM_VOICES; i++) {
            patterns[i] = Pattern(length);
        }
    }

    Pattern& getPattern(int voiceIdx) {
        if (voiceIdx < 0 || voiceIdx >= NUM_VOICES) return patterns[0];
        return patterns[voiceIdx];
    }

    const Pattern& getPattern(int voiceIdx) const {
        if (voiceIdx < 0 || voiceIdx >= NUM_VOICES) return patterns[0];
        return patterns[voiceIdx];
    }
};

/**
 * Pattern 生成器 - 套用 UniversalRhythm 設計
 */
class PatternGenerator {
public:
    PatternGenerator() : rng_(std::random_device{}()) {}

    void seed(unsigned int s) { rng_.seed(s); }

    /**
     * 生成完整的 8 聲道 Pattern
     * @param densities 可選的 per-role density 陣列，nullptr 時使用 variation 計算
     */
    MultiVoicePatterns generate(int length = 16, float variation = 0.5f,
                                const float* densities = nullptr) {
        MultiVoicePatterns result(length);

        // 計算各 Role 的 density
        float localDensities[NUM_ROLES];
        if (densities) {
            for (int r = 0; r < NUM_ROLES; r++) {
                localDensities[r] = densities[r];
            }
        } else {
            // 從當前風格取得範圍，用 variation 計算
            for (int r = 0; r < NUM_ROLES; r++) {
                float dMin = StyleWeights::getDensityMin(static_cast<Role>(r));
                float dMax = StyleWeights::getDensityMax(static_cast<Role>(r));
                localDensities[r] = dMin + variation * (dMax - dMin);
            }
        }

        // 1. Timeline (Voice 0: Primary, Voice 1: Secondary)
        result.patterns[0] = generatePrimary(TIMELINE, length, localDensities[TIMELINE], variation);
        result.patterns[1] = generateWithInterlock(TIMELINE, length, localDensities[TIMELINE] * 0.5f,
                                                   variation + 0.2f, result.patterns[0]);

        // 2. Foundation (Voice 2: Primary, Voice 3: Secondary)
        result.patterns[2] = generateFoundation(length, localDensities[FOUNDATION], variation);
        result.patterns[3] = generateWithInterlock(FOUNDATION, length, localDensities[FOUNDATION] * 0.5f,
                                                   variation + 0.2f, result.patterns[2]);

        // 3. Groove (Voice 4: Primary, Voice 5: Secondary)
        result.patterns[4] = generateGroove(length, localDensities[GROOVE], variation);
        result.patterns[5] = generateWithInterlock(GROOVE, length, localDensities[GROOVE] * 0.6f,
                                                   variation + 0.2f, result.patterns[4]);

        // 4. Lead (Voice 6: Primary, Voice 7: Secondary)
        result.patterns[6] = generatePrimary(LEAD, length, localDensities[LEAD], variation);
        result.patterns[7] = generateWithInterlock(LEAD, length, localDensities[LEAD] * 0.5f,
                                                   variation + 0.2f, result.patterns[6]);

        return result;
    }

private:
    /**
     * 通用 Primary pattern 生成（權重式選擇）
     */
    Pattern generatePrimary(Role role, int length, float density, float variation) {
        Pattern p(length);
        if (density < 0.01f) return p;

        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        std::uniform_real_distribution<float> velVar(-0.1f, 0.1f);

        const float* styleWeights = StyleWeights::getWeights(role);

        // 建立權重陣列
        std::vector<float> weights(static_cast<size_t>(length));
        for (int i = 0; i < length; i++) {
            int mapped = (i * 16) / length;
            weights[static_cast<size_t>(i)] = styleWeights[mapped];
            // 套用 variation（與 uniform 混合）
            weights[static_cast<size_t>(i)] = weights[static_cast<size_t>(i)] * (1.0f - variation) + variation;
        }

        // 權重式隨機選擇
        int targetOnsets = static_cast<int>(std::round(length * density));
        weightedSelect(p, weights, targetOnsets, velVar, dist);

        return p;
    }

    /**
     * Foundation: Skeleton + Variation
     */
    Pattern generateFoundation(int length, float density, float variation) {
        Pattern p(length);
        if (density < 0.01f) return p;

        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        std::uniform_real_distribution<float> velVar(-0.05f, 0.05f);

        int quarterInterval = length / 4;

        // Skeleton: 固定四拍 (beat 1, 2, 3, 4)
        for (int i = 0; i < 4; i++) {
            float vel = 0.9f + velVar(rng_);
            p.setOnset(i * quarterInterval, std::clamp(vel, 0.85f, 1.0f));
        }

        // 高 variation 時加入 off-beat
        if (variation > 0.3f) {
            const float* styleWeights = StyleWeights::getWeights(FOUNDATION);
            for (int i = 0; i < length; i++) {
                if (p.hasOnset(i)) continue;
                int mapped = (i * 16) / length;
                float weight = styleWeights[mapped] * variation;
                if (dist(rng_) < weight * 0.3f) {
                    p.setOnset(i, 0.5f + velVar(rng_));
                }
            }
        }

        return p;
    }

    /**
     * Groove: Clap on 2 and 4 + variations
     */
    Pattern generateGroove(int length, float density, float variation) {
        Pattern p(length);
        if (density < 0.01f) return p;

        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        std::uniform_real_distribution<float> velVar(-0.08f, 0.08f);

        int quarterInterval = length / 4;

        // 固定 clap 在 beat 2 和 4
        p.setOnset(quarterInterval, 0.85f + velVar(rng_));
        p.setOnset(3 * quarterInterval, 0.85f + velVar(rng_));

        // 高 variation 時加入 syncopation
        if (variation > 0.4f) {
            for (int i = 0; i < length; i++) {
                if (p.hasOnset(i)) continue;
                // 只在 off-beat 位置
                if ((i % 4 == 3) && dist(rng_) < variation * 0.25f) {
                    p.setOnset(i, 0.4f + velVar(rng_));
                }
            }
        }

        return p;
    }

    /**
     * Interlock 生成：與 reference 互補的 pattern
     * 來自 UniversalRhythm PatternGenerator::generateWithInterlock
     */
    Pattern generateWithInterlock(Role role, int length, float density, float variation,
                                  const Pattern& reference) {
        Pattern p(length);
        if (density < 0.01f) return p;

        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        std::uniform_real_distribution<float> velVar(-0.1f, 0.1f);

        const float* styleWeights = StyleWeights::getWeights(role);

        // 建立權重陣列
        std::vector<float> weights(static_cast<size_t>(length));
        for (int i = 0; i < length; i++) {
            int mapped = (i * 16) / length;
            weights[static_cast<size_t>(i)] = styleWeights[mapped];
            weights[static_cast<size_t>(i)] = weights[static_cast<size_t>(i)] * (1.0f - variation) + variation;

            // === Interlock 規則 ===
            // Reference 有 onset 的位置：降低權重
            if (reference.hasOnset(i)) {
                weights[static_cast<size_t>(i)] *= 0.2f;
            }

            // Reference 的相鄰位置：提升權重
            int prev = (i - 1 + length) % length;
            int next = (i + 1) % length;
            if (reference.hasOnset(prev) || reference.hasOnset(next)) {
                weights[static_cast<size_t>(i)] *= 1.3f;
            }
        }

        // 權重式隨機選擇
        int targetOnsets = static_cast<int>(std::round(length * density));
        weightedSelect(p, weights, targetOnsets, velVar, dist);

        return p;
    }

    /**
     * 權重式隨機選擇
     */
    void weightedSelect(Pattern& p, std::vector<float>& weights, int targetOnsets,
                        std::uniform_real_distribution<float>& velVar,
                        std::uniform_real_distribution<float>& dist) {
        int length = p.length;
        int placed = 0;

        while (placed < targetOnsets) {
            // 計算總權重
            float totalWeight = 0.0f;
            for (int i = 0; i < length; i++) {
                if (!p.hasOnset(i)) {
                    totalWeight += weights[static_cast<size_t>(i)];
                }
            }

            if (totalWeight < 0.001f) break;

            // 隨機選擇位置
            float rand = dist(rng_) * totalWeight;
            float cumulative = 0.0f;
            int selected = -1;

            for (int i = 0; i < length; i++) {
                if (!p.hasOnset(i)) {
                    cumulative += weights[static_cast<size_t>(i)];
                    if (cumulative >= rand) {
                        selected = i;
                        break;
                    }
                }
            }

            if (selected >= 0) {
                float vel = 0.6f + weights[static_cast<size_t>(selected)] * 0.3f + velVar(rng_);
                p.setOnset(selected, std::clamp(vel, 0.3f, 1.0f));
                weights[static_cast<size_t>(selected)] = 0.0f;  // 防止重複選擇
                placed++;
            } else {
                break;
            }
        }
    }

    std::mt19937 rng_;
};

/**
 * Synth Modifier - Variation 影響音色參數
 */
struct SynthModifiers {
    float freqMod[NUM_VOICES];   // 頻率乘數 (0.5 - 2.0)
    float decayMod[NUM_VOICES];  // Decay 乘數 (0.2 - 2.0)

    SynthModifiers() {
        for (int i = 0; i < NUM_VOICES; i++) {
            freqMod[i] = 1.0f;
            decayMod[i] = 1.0f;
        }
    }
};

/**
 * Crossfade 觸發決策結果
 */
struct CrossfadeDecision {
    bool shouldTrigger;
    float velocity;
};

/**
 * 混合後的音色預設（用於 crossfader 混合兩個風格）
 */
struct MixedPreset {
    SynthMode mode[NUM_VOICES];
    float freq[NUM_VOICES];
    float decay[NUM_VOICES];
};

/**
 * Techno Pattern 引擎
 */
class TechnoPatternEngine {
public:
    TechnoPatternEngine() : rng_(std::random_device{}()) {}

    // 風格切換（統一風格）- 設定到當前作用中的 Deck
    void setStyle(int styleIdx) {
        if (styleIdx >= 0 && styleIdx < NUM_STYLES) {
            currentStyleIdx_ = styleIdx;
            Deck& d = (crossfaderPosition_ < 0.5f) ? deckA_ : deckB_;
            for (int i = 0; i < NUM_ROLES; i++) {
                d.styleIndices[i] = styleIdx;
            }
            StyleWeights::setStyle(styleIdx);
        }
    }

    void setStyle(StyleType style) {
        setStyle(static_cast<int>(style));
    }

    // 設定複合風格到當前作用中的 Deck
    void setCompositeStyle(const int* roleStyles) {
        Deck& d = (crossfaderPosition_ < 0.5f) ? deckA_ : deckB_;
        for (int i = 0; i < NUM_ROLES; i++) {
            d.styleIndices[i] = roleStyles[i];
        }
        StyleWeights::setCompositeStyle(roleStyles);
        currentStyleIdx_ = d.styleIndices[0];
    }

    // 設定複合風格到指定 Deck
    void setDeckCompositeStyle(int deck, const int* roleStyles) {
        Deck& d = getDeck(deck);
        for (int i = 0; i < NUM_ROLES; i++) {
            d.styleIndices[i] = roleStyles[i];
        }
    }

    int getStyleIdx() const { return currentStyleIdx_; }
    int getRoleStyleIdx(Role role) const {
        if (role >= 0 && role < NUM_ROLES) {
            const Deck& d = (crossfaderPosition_ < 0.5f) ? deckA_ : deckB_;
            return d.styleIndices[role];
        }
        return 0;
    }

    /**
     * 取得混合後的 Swing 值
     * 根據 crossfader 位置混合 Deck A 和 B 的風格 swing
     */
    float getMixedSwing() const {
        float djPos = applyDJCurve(crossfaderPosition_);

        // 取得各 Deck 主要風格的 swing（使用 Foundation 的風格作為代表）
        float swingA = STYLES[deckA_.styleIndices[FOUNDATION]]->swing;
        float swingB = STYLES[deckB_.styleIndices[FOUNDATION]]->swing;

        return swingA * (1.0f - djPos) + swingB * djPos;
    }

    // 取得當前作用中 Deck 的風格名稱
    const char* getStyleName() const {
        return getDeckStyleName((crossfaderPosition_ < 0.5f) ? 0 : 1);
    }

    // Density 控制
    void setDensity(Role role, float density) {
        if (role >= 0 && role < NUM_ROLES) {
            roleDensities_[role] = std::clamp(density, 0.0f, 0.9f);
        }
    }

    float getDensity(Role role) const {
        if (role >= 0 && role < NUM_ROLES) {
            return roleDensities_[role];
        }
        return 0.5f;
    }

    /**
     * 重新生成 patterns（相容舊介面）
     * 生成到 Deck A 並設定 crossfader 為 0
     */
    void regenerate(int length = 16, float variation = 0.5f) {
        patternLength_ = length;

        // 使用當前風格設定
        int styles[NUM_ROLES];
        for (int i = 0; i < NUM_ROLES; i++) {
            styles[i] = deckA_.styleIndices[i];
        }

        // 生成到 Deck A
        loadToDeck(0, styles, variation);

        // 確保 crossfader 在 A
        crossfaderPosition_ = 0.0f;
    }

    /**
     * 初始化兩個 Deck（首次啟動時使用）
     */
    void initializeDecks(int length, float variationA, float variationB) {
        patternLength_ = length;

        // Deck A 使用預設 Techno
        int defaultStyle[NUM_ROLES] = {0, 0, 0, 0};
        loadToDeck(0, defaultStyle, variationA);

        // Deck B 也初始化（使用不同 variation）
        loadToDeck(1, defaultStyle, variationB);

        crossfaderPosition_ = 0.0f;
    }

    // 相容舊介面：取得當前作用中 deck 的 patterns
    const MultiVoicePatterns& patterns() const {
        return (crossfaderPosition_ < 0.5f) ? deckA_.patterns : deckB_.patterns;
    }
    MultiVoicePatterns& patterns() {
        return (crossfaderPosition_ < 0.5f) ? deckA_.patterns : deckB_.patterns;
    }

    const Pattern& getPattern(int voiceIdx) const {
        return patterns().getPattern(voiceIdx);
    }

    float getVariation() const {
        return (crossfaderPosition_ < 0.5f) ? deckA_.variation : deckB_.variation;
    }

    // 取得混合後的 SynthModifiers（用於音色控制）
    const SynthModifiers& getSynthModifiers() const {
        // 注意：這是靜態相容介面，實際應使用 getMixedSynthModifiers()
        return (crossfaderPosition_ < 0.5f) ? deckA_.synthMods : deckB_.synthMods;
    }

    // Fill 系統
    void setFillInterval(int bars) { fillInterval_ = std::max(1, bars); }
    int getFillInterval() const { return fillInterval_; }

    void notifyBarStart(int barNumber) {
        // 每 N bars 觸發 Fill
        if (fillInterval_ > 0 && barNumber > 0 && (barNumber % fillInterval_) == (fillInterval_ - 1)) {
            fillActive_ = true;
            fillStepsRemaining_ = patternLength_;
        }
    }

    bool isFillActive() const { return fillActive_; }

    // 取得當前作用中的 pattern（考慮 Fill）
    const Pattern& getActivePattern(int voiceIdx) const {
        const Deck& d = (crossfaderPosition_ < 0.5f) ? deckA_ : deckB_;
        if (fillActive_ && voiceIdx >= 0 && voiceIdx < NUM_VOICES) {
            return d.fillPatterns.getPattern(voiceIdx);
        }
        return d.patterns.getPattern(voiceIdx);
    }

    void advanceStep() {
        if (fillActive_ && fillStepsRemaining_ > 0) {
            fillStepsRemaining_--;
            if (fillStepsRemaining_ <= 0) {
                fillActive_ = false;
            }
        }
    }

    // === DJ Deck A/B 手動混音系統 ===

    /**
     * 設定 Crossfader 位置（手動控制）
     * @param position 0.0 = 全 Deck A，1.0 = 全 Deck B
     */
    void setCrossfader(float position) {
        crossfaderPosition_ = std::clamp(position, 0.0f, 1.0f);
    }

    float getCrossfader() const { return crossfaderPosition_; }

    /**
     * 載入歌曲到指定 Deck
     * @param deck 0 = Deck A, 1 = Deck B
     * @param roleStyles 複合風格陣列
     * @param variation 變化度
     */
    void loadToDeck(int deck, const int* roleStyles, float variation) {
        Deck& d = getDeck(deck);

        // 複製風格設定
        for (int i = 0; i < NUM_ROLES; i++) {
            d.styleIndices[i] = roleStyles[i];
        }

        // 生成 patterns
        generateDeckPatterns(d, patternLength_, variation);
    }

    /**
     * 載入下一首歌到非作用中的 Deck
     * 自動決定目標 Deck（根據 crossfader 位置）
     */
    void loadNextSong(const int* roleStyles, float variation) {
        // 根據 crossfader 決定載入到哪個 deck
        // crossfader < 0.5 時載入到 B（準備切換過去）
        // crossfader >= 0.5 時載入到 A（準備切換回來）
        int targetDeck = (crossfaderPosition_ < 0.5f) ? 1 : 0;
        loadToDeck(targetDeck, roleStyles, variation);
    }

    /**
     * 取得當前作用中 Deck（根據 crossfader 位置）
     */
    int getActiveDeck() const {
        return (crossfaderPosition_ < 0.5f) ? 0 : 1;
    }

    /**
     * 取得指定 Deck 的風格名稱
     */
    const char* getDeckStyleName(int deck) const {
        const Deck& d = getDeck(deck);
        // 檢查是否為複合風格
        bool isComposite = false;
        for (int i = 1; i < NUM_ROLES; i++) {
            if (d.styleIndices[i] != d.styleIndices[0]) {
                isComposite = true;
                break;
            }
        }
        if (isComposite) {
            return "Mixed";
        }
        return TechnoMachine::getStyleName(d.styleIndices[0]);
    }

    /**
     * 取得指定 Deck 的特定角色風格名稱
     */
    const char* getDeckRoleStyleName(int deck, Role role) const {
        const Deck& d = getDeck(deck);
        if (role >= 0 && role < NUM_ROLES) {
            return TechnoMachine::getStyleName(d.styleIndices[role]);
        }
        return "Unknown";
    }

    /**
     * 取得指定 Deck 的特定角色風格索引
     */
    int getDeckRoleStyleIdx(int deck, Role role) const {
        const Deck& d = getDeck(deck);
        if (role >= 0 && role < NUM_ROLES) {
            return d.styleIndices[role];
        }
        return 0;
    }

    /**
     * 取得混合後的音色預設
     * 根據 crossfader 位置平滑混合 Deck A 和 B 的風格預設
     * 所有角色都使用平滑過渡
     */
    MixedPreset getMixedPresets() const {
        MixedPreset result;
        float djPos = applyDJCurve(crossfaderPosition_);

        for (int v = 0; v < NUM_VOICES; v++) {
            int role = v / 2;

            // 取得各 Deck 對應角色的風格預設
            int styleA = deckA_.styleIndices[role];
            int styleB = deckB_.styleIndices[role];
            const VoicePreset& presetA = STYLE_PRESETS[styleA][v];
            const VoicePreset& presetB = STYLE_PRESETS[styleB][v];

            // 套用 Deck 的 variation 修正
            float freqA = presetA.freq * deckA_.synthMods.freqMod[v];
            float decayA = presetA.decay * deckA_.synthMods.decayMod[v];
            float freqB = presetB.freq * deckB_.synthMods.freqMod[v];
            float decayB = presetB.decay * deckB_.synthMods.decayMod[v];

            // 所有角色都平滑混合
            result.mode[v] = (djPos < 0.5f) ? presetA.mode : presetB.mode;
            result.freq[v] = freqA * (1.0f - djPos) + freqB * djPos;
            result.decay[v] = decayA * (1.0f - djPos) + decayB * djPos;
        }

        return result;
    }

    // 保留舊介面相容性
    SynthModifiers getMixedSynthModifiers() const {
        SynthModifiers result;
        for (int v = 0; v < NUM_VOICES; v++) {
            result.freqMod[v] = 1.0f;
            result.decayMod[v] = 1.0f;
        }
        return result;
    }

    /**
     * 取得混音決策
     * 根據 crossfader 位置決定播放哪個 pattern 的音符
     * 所有角色都使用機率混合（DJ 曲線控制）
     */
    CrossfadeDecision getMixDecision(int voiceIdx, int step) {
        CrossfadeDecision result = {false, 0.0f};

        float djPos = applyDJCurve(crossfaderPosition_);

        // 選擇 pattern（考慮 Fill）
        const Pattern& patA = fillActive_ ?
            deckA_.fillPatterns.getPattern(voiceIdx) :
            deckA_.patterns.getPattern(voiceIdx);
        const Pattern& patB = fillActive_ ?
            deckB_.fillPatterns.getPattern(voiceIdx) :
            deckB_.patterns.getPattern(voiceIdx);

        bool hasA = patA.hasOnset(step);
        bool hasB = patB.hasOnset(step);

        // 所有角色：機率混合
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        float randVal = dist(rng_);

        float weightA = 1.0f - djPos;
        float weightB = djPos;

        if (hasA && hasB) {
            // 兩邊都有：根據權重選擇其一
            if (randVal < weightB) {
                result.shouldTrigger = true;
                result.velocity = patB.getVelocity(step);
            } else {
                result.shouldTrigger = true;
                result.velocity = patA.getVelocity(step);
            }
        } else if (hasA) {
            // 只有 A：機率 = weightA
            if (randVal < weightA) {
                result.shouldTrigger = true;
                result.velocity = patA.getVelocity(step);
            }
        } else if (hasB) {
            // 只有 B：機率 = weightB
            if (randVal < weightB) {
                result.shouldTrigger = true;
                result.velocity = patB.getVelocity(step);
            }
        }

        return result;
    }

    // 相容舊介面
    bool isCrossfading() const {
        return crossfaderPosition_ > 0.01f && crossfaderPosition_ < 0.99f;
    }
    float getCrossfadeProgress() const { return crossfaderPosition_; }
    void notifyCrossfadeBarStart() { /* 手動模式不需要 */ }
    void startCrossfade(int /*durationBars*/, float newVariation) {
        // 相容舊介面：載入到另一個 deck
        int targetDeck = (crossfaderPosition_ < 0.5f) ? 1 : 0;
        Deck& d = getDeck(targetDeck);
        generateDeckPatterns(d, patternLength_, newVariation);
    }

    // 已棄用，改用 getMixDecision
    CrossfadeDecision getCrossfadeDecision(int voiceIdx, int step) {
        return getMixDecision(voiceIdx, step);
    }

private:
    PatternGenerator generator_;
    int patternLength_ = 16;

    // === Deck A/B 雙軌系統 ===
    struct Deck {
        MultiVoicePatterns patterns{16};
        MultiVoicePatterns fillPatterns{16};
        SynthModifiers synthMods;
        int styleIndices[NUM_ROLES] = {0, 0, 0, 0};
        float variation = 0.5f;

        void clear() {
            for (int i = 0; i < NUM_VOICES; i++) {
                patterns.patterns[i].clear();
                fillPatterns.patterns[i].clear();
            }
        }
    };

    Deck deckA_;
    Deck deckB_;
    int activeDeck_ = 0;  // 0 = A, 1 = B（用於載入新歌時決定目標）

    // 手動 Crossfader（0.0 = 全 A，1.0 = 全 B）
    float crossfaderPosition_ = 0.0f;

    // Per-role density（0.0 - 0.9）- 全域設定
    float roleDensities_[NUM_ROLES] = {0.4f, 0.2f, 0.5f, 0.5f};

    // Fill 狀態
    int fillInterval_ = 4;
    bool fillActive_ = false;
    int fillStepsRemaining_ = 0;

    // 相容舊介面
    int currentStyleIdx_ = 0;

    std::mt19937 rng_;

    /**
     * DJ 風格 Crossfader 曲線
     * 中間凹陷，兩端陡峭 - 模擬真實 DJ 混音台行為
     *
     * 特性：
     * - 在 0.0 和 1.0 附近變化快（快速切入/切出）
     * - 在 0.5 附近變化慢（方便微調混合比例）
     *
     * 使用修改過的 S-curve: 讓中間區域更平坦
     */
    float applyDJCurve(float t) const {
        t = std::clamp(t, 0.0f, 1.0f);

        // 使用 quintic smoothstep 讓曲線更陡峭
        // 6t^5 - 15t^4 + 10t^3
        float t3 = t * t * t;
        float t4 = t3 * t;
        float t5 = t4 * t;
        return 6.0f * t5 - 15.0f * t4 + 10.0f * t3;
    }

    /**
     * 取得指定 Deck 的參考
     */
    Deck& getDeck(int deck) {
        return (deck == 0) ? deckA_ : deckB_;
    }

    const Deck& getDeck(int deck) const {
        return (deck == 0) ? deckA_ : deckB_;
    }

    /**
     * 內部：生成指定 Deck 的 patterns
     */
    void generateDeckPatterns(Deck& deck, int length, float variation) {
        // 設定風格權重
        StyleWeights::setCompositeStyle(deck.styleIndices);

        // 生成 patterns
        deck.patterns = generator_.generate(length, variation, roleDensities_);
        deck.variation = variation;

        // 加入 Ghost Notes
        addGhostNotesToDeck(deck, variation);

        // 生成 Synth Modifiers
        generateDeckSynthModifiers(deck, variation);

        // 生成 Fill Pattern
        generateDeckFillPattern(deck, length, variation);
    }

    /**
     * 為指定 Deck 加入 Ghost Notes
     */
    void addGhostNotesToDeck(Deck& deck, float variation) {
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        std::uniform_real_distribution<float> velDist(0.25f, 0.32f);
        float ghostProb = 0.1f + variation * 0.2f;

        for (int v = 0; v < NUM_VOICES; v++) {
            Pattern& p = deck.patterns.patterns[v];
            for (int i = 0; i < p.length; i++) {
                if (p.hasOnset(i)) continue;

                int prev = (i - 1 + p.length) % p.length;
                int next = (i + 1) % p.length;
                bool nearHit = p.hasOnset(prev) || p.hasOnset(next);
                bool isWeakBeat = (i % 2 == 1);

                float prob = ghostProb;
                if (nearHit) prob *= 2.0f;
                if (isWeakBeat) prob *= 1.5f;

                if (dist(rng_) < prob) {
                    p.setOnset(i, velDist(rng_));
                }
            }
        }
    }

    /**
     * 為指定 Deck 生成 Synth Modifiers
     */
    void generateDeckSynthModifiers(Deck& deck, float variation) {
        std::uniform_real_distribution<float> freqVar(-0.3f, 0.3f);
        std::uniform_real_distribution<float> decayVar(-0.2f, 0.2f);

        for (int v = 0; v < NUM_VOICES; v++) {
            float freqBase = 1.0f + (variation - 0.5f) * 0.4f;
            float decayBase = 1.0f + (variation - 0.5f) * 0.3f;

            deck.synthMods.freqMod[v] = std::clamp(freqBase + freqVar(rng_) * variation, 0.5f, 2.0f);
            deck.synthMods.decayMod[v] = std::clamp(decayBase + decayVar(rng_) * variation, 0.2f, 2.0f);
        }
    }

    /**
     * 為指定 Deck 生成 Fill Pattern
     */
    void generateDeckFillPattern(Deck& deck, int length, float variation) {
        deck.fillPatterns = generator_.generate(length, variation + 0.2f, roleDensities_);

        std::uniform_real_distribution<float> velDist(0.7f, 1.0f);
        for (int v = 0; v < NUM_VOICES; v++) {
            Pattern& fill = deck.fillPatterns.patterns[v];
            for (int i = 0; i < fill.length; i++) {
                if (fill.hasOnset(i)) {
                    fill.setOnset(i, velDist(rng_));
                }
            }
        }
    }

};  // class TechnoPatternEngine

} // namespace TechnoMachine
