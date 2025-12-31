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
 * 從 StyleProfile 動態取得權重
 */
class StyleWeights {
public:
    static void setStyle(const StyleProfile* style) {
        currentStyle_ = style;
    }

    static const StyleProfile* getStyle() {
        return currentStyle_ ? currentStyle_ : &STYLE_TECHNO;
    }

    static const float* getWeights(Role role) {
        const StyleProfile* s = getStyle();
        switch (role) {
            case TIMELINE: return s->timeline;
            case FOUNDATION: return s->foundation;
            case GROOVE: return s->groove;
            case LEAD: return s->lead;
            default: return s->timeline;
        }
    }

    static float getDensityMin(Role role) {
        return getStyle()->densityRange[role][0];
    }

    static float getDensityMax(Role role) {
        return getStyle()->densityRange[role][1];
    }

private:
    static inline const StyleProfile* currentStyle_ = &STYLE_TECHNO;
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
 * Techno Pattern 引擎
 */
class TechnoPatternEngine {
public:
    TechnoPatternEngine() : rng_(std::random_device{}()) {}

    // 風格切換
    void setStyle(int styleIdx) {
        if (styleIdx >= 0 && styleIdx < NUM_STYLES) {
            currentStyleIdx_ = styleIdx;
            StyleWeights::setStyle(STYLES[styleIdx]);
        }
    }

    void setStyle(StyleType style) {
        setStyle(static_cast<int>(style));
    }

    int getStyleIdx() const { return currentStyleIdx_; }
    const char* getStyleName() const { return TechnoMachine::getStyleName(currentStyleIdx_); }

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

    void regenerate(int length = 16, float variation = 0.5f) {
        patterns_ = generator_.generate(length, variation, roleDensities_);
        currentVariation_ = variation;
        patternLength_ = length;

        // 套用 Ghost Notes
        addGhostNotes(variation);

        // 生成 Synth Modifiers
        generateSynthModifiers(variation);

        // 生成 Fill Pattern
        generateFillPattern(length, variation);
    }

    const MultiVoicePatterns& patterns() const { return patterns_; }
    MultiVoicePatterns& patterns() { return patterns_; }

    const Pattern& getPattern(int voiceIdx) const {
        return patterns_.getPattern(voiceIdx);
    }

    float getVariation() const { return currentVariation_; }
    const SynthModifiers& getSynthModifiers() const { return synthMods_; }

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

    const Pattern& getActivePattern(int voiceIdx) const {
        if (fillActive_ && voiceIdx >= 0 && voiceIdx < NUM_VOICES) {
            return fillPatterns_.getPattern(voiceIdx);
        }
        return patterns_.getPattern(voiceIdx);
    }

    void advanceStep() {
        if (fillActive_ && fillStepsRemaining_ > 0) {
            fillStepsRemaining_--;
            if (fillStepsRemaining_ <= 0) {
                fillActive_ = false;
            }
        }
    }

    // === DJ Crossfade 系統 ===

    /**
     * 開始 crossfade 過渡
     * 儲存當前 patterns 作為 outgoing，生成新 patterns 作為 incoming
     */
    void startCrossfade(int durationBars, float newVariation) {
        // 儲存當前 patterns 作為 outgoing
        outgoingPatterns_ = patterns_;

        // 生成新 patterns
        regenerate(patternLength_, newVariation);

        // 設定 crossfade 狀態
        crossfadeDurationBars_ = std::max(1, durationBars);
        crossfadeBarCount_ = 0;
        crossfadeProgress_ = 0.0f;
        isCrossfading_ = true;
    }

    /**
     * 通知 crossfade 小節開始
     */
    void notifyCrossfadeBarStart() {
        if (!isCrossfading_) return;

        crossfadeBarCount_++;
        crossfadeProgress_ = static_cast<float>(crossfadeBarCount_) /
                            static_cast<float>(crossfadeDurationBars_);

        if (crossfadeProgress_ >= 1.0f) {
            crossfadeProgress_ = 1.0f;
            isCrossfading_ = false;
        }
    }

    bool isCrossfading() const { return isCrossfading_; }
    float getCrossfadeProgress() const { return crossfadeProgress_; }

    /**
     * 取得 crossfade 觸發決策（機率混合）
     * 使用 Equal Power 曲線混合 onset 機率
     *
     * @param voiceIdx 聲道索引
     * @param step 當前 step
     * @return 是否觸發及 velocity
     */
    CrossfadeDecision getCrossfadeDecision(int voiceIdx, int step) {
        CrossfadeDecision result = {false, 0.0f};

        if (!isCrossfading_) {
            // 不在 crossfade 中，使用正常 pattern
            const Pattern& p = getActivePattern(voiceIdx);
            if (p.hasOnset(step)) {
                result.shouldTrigger = true;
                result.velocity = p.getVelocity(step);
            }
            return result;
        }

        // 計算 Equal Power 權重
        float t = crossfadeProgress_;
        float outWeight = static_cast<float>(std::cos(t * M_PI * 0.5));  // cos(0) = 1, cos(π/2) = 0
        float inWeight = static_cast<float>(std::sin(t * M_PI * 0.5));   // sin(0) = 0, sin(π/2) = 1

        // Foundation role（Voice 2, 3）使用 hard swap
        int role = voiceIdx / 2;
        if (role == FOUNDATION) {
            return getFoundationSwapDecision(voiceIdx, step, t);
        }

        // 其他 role 使用機率混合
        const Pattern& outPattern = outgoingPatterns_.getPattern(voiceIdx);
        const Pattern& inPattern = patterns_.getPattern(voiceIdx);

        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        float randVal = dist(rng_);

        bool outHas = outPattern.hasOnset(step);
        bool inHas = inPattern.hasOnset(step);

        if (outHas && inHas) {
            // 兩個都有 onset：混合 velocity
            float outVel = outPattern.getVelocity(step) * outWeight;
            float inVel = inPattern.getVelocity(step) * inWeight;
            result.shouldTrigger = true;
            result.velocity = outVel + inVel;
        }
        else if (outHas) {
            // 只有 outgoing 有：機率隨 outWeight 降低
            if (randVal < outWeight) {
                result.shouldTrigger = true;
                result.velocity = outPattern.getVelocity(step) * outWeight;
            }
        }
        else if (inHas) {
            // 只有 incoming 有：機率隨 inWeight 提升
            if (randVal < inWeight) {
                result.shouldTrigger = true;
                result.velocity = inPattern.getVelocity(step) * inWeight;
            }
        }

        return result;
    }

private:
    PatternGenerator generator_;
    MultiVoicePatterns patterns_;
    MultiVoicePatterns fillPatterns_;
    SynthModifiers synthMods_;
    float currentVariation_ = 0.5f;
    int patternLength_ = 16;
    int currentStyleIdx_ = 0;  // 預設 Techno

    // Per-role density（0.0 - 0.9）
    float roleDensities_[NUM_ROLES] = {0.4f, 0.2f, 0.5f, 0.5f};  // Timeline, Foundation, Groove, Lead

    // Fill 狀態
    int fillInterval_ = 4;  // 預設每 4 bars
    bool fillActive_ = false;
    int fillStepsRemaining_ = 0;

    // Crossfade 狀態
    MultiVoicePatterns outgoingPatterns_;
    bool isCrossfading_ = false;
    float crossfadeProgress_ = 0.0f;
    int crossfadeDurationBars_ = 8;
    int crossfadeBarCount_ = 0;

    std::mt19937 rng_;

    /**
     * Foundation（Kick）的 hard swap 決策
     * 不同時播放兩個 kick pattern，在 50% 進度時瞬間切換
     */
    CrossfadeDecision getFoundationSwapDecision(int voiceIdx, int step, float progress) {
        CrossfadeDecision result = {false, 0.0f};

        const Pattern& outPattern = outgoingPatterns_.getPattern(voiceIdx);
        const Pattern& inPattern = patterns_.getPattern(voiceIdx);

        if (progress < 0.5f) {
            // 前半段：只播放 outgoing
            if (outPattern.hasOnset(step)) {
                result.shouldTrigger = true;
                result.velocity = outPattern.getVelocity(step);
            }
        } else {
            // 後半段：只播放 incoming
            if (inPattern.hasOnset(step)) {
                result.shouldTrigger = true;
                result.velocity = inPattern.getVelocity(step);
            }
        }

        return result;
    }

    /**
     * 加入 Ghost Notes - 低 velocity 裝飾音
     * 來自 UniversalRhythm addGhostNotes()
     */
    void addGhostNotes(float variation) {
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        std::uniform_real_distribution<float> velDist(0.25f, 0.32f);

        // Ghost note 機率隨 variation 增加
        float ghostProb = 0.1f + variation * 0.2f;

        for (int v = 0; v < NUM_VOICES; v++) {
            Pattern& p = patterns_.patterns[v];

            for (int i = 0; i < p.length; i++) {
                // 只在沒有 onset 的位置加入
                if (p.hasOnset(i)) continue;

                // 檢查相鄰是否有 onset（ghost note 通常在主音附近）
                int prev = (i - 1 + p.length) % p.length;
                int next = (i + 1) % p.length;

                bool nearHit = p.hasOnset(prev) || p.hasOnset(next);

                // 弱拍位置 + 附近有主音 = 較高機率
                bool weakBeat = (i % 4 != 0);

                if (weakBeat && nearHit && dist(rng_) < ghostProb) {
                    p.setOnset(i, velDist(rng_));
                }
            }
        }
    }

    /**
     * 生成 Synth Modifiers - Variation 影響音色
     */
    void generateSynthModifiers(float variation) {
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);

        for (int v = 0; v < NUM_VOICES; v++) {
            // 頻率變化：variation 高時變化幅度大 (0.7 - 1.4)
            float freqRange = 0.3f * variation;
            synthMods_.freqMod[v] = 1.0f + (dist(rng_) - 0.5f) * freqRange * 2.0f;

            // Decay 變化：(0.5 - 1.5)
            float decayRange = 0.5f * variation;
            synthMods_.decayMod[v] = 1.0f + (dist(rng_) - 0.5f) * decayRange * 2.0f;
        }
    }

    /**
     * 生成 Fill Pattern - 過門時的高密度 pattern
     */
    void generateFillPattern(int length, float variation) {
        fillPatterns_ = MultiVoicePatterns(length);

        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        std::uniform_real_distribution<float> velDist(0.6f, 1.0f);

        // Fill 使用更高密度
        float fillDensityBoost = 1.5f + variation * 0.5f;

        for (int v = 0; v < NUM_VOICES; v++) {
            Pattern& fillP = fillPatterns_.patterns[v];
            const Pattern& normalP = patterns_.patterns[v];

            // 複製原始 pattern
            for (int i = 0; i < length; i++) {
                if (normalP.hasOnset(i)) {
                    fillP.setOnset(i, normalP.getVelocity(i));
                }
            }

            // 加入額外的 fill 觸發
            int role = v / 2;
            float baseDensity = roleDensities_[role];
            float fillDensity = std::min(0.9f, baseDensity * fillDensityBoost);

            int extraOnsets = static_cast<int>((fillDensity - baseDensity) * length);

            for (int e = 0; e < extraOnsets; e++) {
                int pos = static_cast<int>(dist(rng_) * length);
                if (!fillP.hasOnset(pos)) {
                    fillP.setOnset(pos, velDist(rng_));
                }
            }

            // Snare roll effect for Groove role (voice 4,5)
            if (role == GROOVE && variation > 0.3f) {
                // 最後 4 步加入密集觸發
                for (int i = length - 4; i < length; i++) {
                    if (dist(rng_) < 0.7f) {
                        fillP.setOnset(i, 0.7f + dist(rng_) * 0.3f);
                    }
                }
            }
        }
    }
};

} // namespace TechnoMachine
