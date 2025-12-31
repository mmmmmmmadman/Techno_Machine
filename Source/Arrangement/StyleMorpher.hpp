/**
 * StyleMorpher.hpp
 * Techno Machine - 風格漸進混合系統
 *
 * 用於 DJ Set 過渡時，在兩種風格間平滑切換
 */

#pragma once

#include "../Sequencer/StyleProfiles.hpp"
#include <cmath>

namespace TechnoMachine {

/**
 * 混合後的風格設定檔
 */
struct MorphedStyle {
    float timeline[16];
    float foundation[16];
    float groove[16];
    float lead[16];
    float densityRange[NUM_ROLES][2];
    float swing;
};

/**
 * 風格漸進混合器
 */
class StyleMorpher {
public:
    StyleMorpher() = default;

    /**
     * 設定起始和目標風格
     */
    void setStyles(int fromStyleIdx, int toStyleIdx) {
        if (fromStyleIdx >= 0 && fromStyleIdx < NUM_STYLES) {
            fromStyle_ = STYLES[fromStyleIdx];
            fromStyleIdx_ = fromStyleIdx;
        }
        if (toStyleIdx >= 0 && toStyleIdx < NUM_STYLES) {
            toStyle_ = STYLES[toStyleIdx];
            toStyleIdx_ = toStyleIdx;
        }
    }

    void setStyles(StyleType from, StyleType to) {
        setStyles(static_cast<int>(from), static_cast<int>(to));
    }

    /**
     * 開始過渡
     * @param durationBars 過渡持續的小節數
     */
    void startTransition(int durationBars) {
        morphDurationBars_ = std::max(1, durationBars);
        morphProgress_ = 0.0f;
        isMorphing_ = true;
    }

    /**
     * 通知小節開始（用於計算過渡進度）
     */
    void notifyBarStart() {
        if (!isMorphing_) return;

        morphProgress_ += 1.0f / static_cast<float>(morphDurationBars_);

        if (morphProgress_ >= 1.0f) {
            morphProgress_ = 1.0f;
            isMorphing_ = false;
            // 過渡完成，更新 from 為 to
            fromStyle_ = toStyle_;
            fromStyleIdx_ = toStyleIdx_;
        }
    }

    /**
     * 取得當前混合進度
     */
    float getMorphProgress() const { return morphProgress_; }
    bool isMorphing() const { return isMorphing_; }

    /**
     * 取得當前混合後的風格
     */
    MorphedStyle getMorphedStyle() const {
        MorphedStyle result;

        float t = smoothStep(morphProgress_);  // 使用平滑曲線

        // 混合權重陣列
        for (int i = 0; i < 16; i++) {
            result.timeline[i] = lerp(fromStyle_->timeline[i], toStyle_->timeline[i], t);
            result.foundation[i] = lerp(fromStyle_->foundation[i], toStyle_->foundation[i], t);
            result.groove[i] = lerp(fromStyle_->groove[i], toStyle_->groove[i], t);
            result.lead[i] = lerp(fromStyle_->lead[i], toStyle_->lead[i], t);
        }

        // 混合 density 範圍
        for (int r = 0; r < NUM_ROLES; r++) {
            result.densityRange[r][0] = lerp(fromStyle_->densityRange[r][0],
                                             toStyle_->densityRange[r][0], t);
            result.densityRange[r][1] = lerp(fromStyle_->densityRange[r][1],
                                             toStyle_->densityRange[r][1], t);
        }

        // 混合 swing
        result.swing = lerp(fromStyle_->swing, toStyle_->swing, t);

        return result;
    }

    /**
     * 取得混合後的權重陣列（供 pattern 生成使用）
     */
    const float* getWeights(Role role) const {
        // 暫存混合結果
        static thread_local float weights[16];

        float t = smoothStep(morphProgress_);

        const float* fromWeights = nullptr;
        const float* toWeights = nullptr;

        switch (role) {
            case TIMELINE:
                fromWeights = fromStyle_->timeline;
                toWeights = toStyle_->timeline;
                break;
            case FOUNDATION:
                fromWeights = fromStyle_->foundation;
                toWeights = toStyle_->foundation;
                break;
            case GROOVE:
                fromWeights = fromStyle_->groove;
                toWeights = toStyle_->groove;
                break;
            case LEAD:
                fromWeights = fromStyle_->lead;
                toWeights = toStyle_->lead;
                break;
            default:
                fromWeights = fromStyle_->timeline;
                toWeights = toStyle_->timeline;
        }

        for (int i = 0; i < 16; i++) {
            weights[i] = lerp(fromWeights[i], toWeights[i], t);
        }

        return weights;
    }

    float getDensityMin(Role role) const {
        float t = smoothStep(morphProgress_);
        return lerp(fromStyle_->densityRange[role][0],
                    toStyle_->densityRange[role][0], t);
    }

    float getDensityMax(Role role) const {
        float t = smoothStep(morphProgress_);
        return lerp(fromStyle_->densityRange[role][1],
                    toStyle_->densityRange[role][1], t);
    }

    // 當前風格資訊
    int getFromStyleIdx() const { return fromStyleIdx_; }
    int getToStyleIdx() const { return toStyleIdx_; }

private:
    const StyleProfile* fromStyle_ = &STYLE_TECHNO;
    const StyleProfile* toStyle_ = &STYLE_TECHNO;
    int fromStyleIdx_ = 0;
    int toStyleIdx_ = 0;

    float morphProgress_ = 0.0f;
    int morphDurationBars_ = 8;
    bool isMorphing_ = false;

    // 線性插值
    static float lerp(float a, float b, float t) {
        return a + (b - a) * t;
    }

    // 平滑曲線（避免突然的變化）
    static float smoothStep(float t) {
        // Hermite 插值：3t² - 2t³
        t = std::clamp(t, 0.0f, 1.0f);
        return t * t * (3.0f - 2.0f * t);
    }
};

} // namespace TechnoMachine
