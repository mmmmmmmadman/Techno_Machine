/**
 * TransitionEngine.hpp
 * Techno Machine - DJ Set 過渡引擎
 *
 * 協調 SongManager 與 StyleMorpher，處理完整的換歌過渡邏輯
 */

#pragma once

#include "SongManager.hpp"
#include "StyleMorpher.hpp"
#include <functional>

namespace TechnoMachine {

/**
 * 過渡狀態
 */
enum class TransitionState {
    IDLE,           // 正常播放
    PREPARING,      // 準備過渡（filter sweep 開始）
    MORPHING,       // 風格混合中
    COMPLETING      // 過渡完成階段
};

/**
 * Filter Sweep 方向
 */
enum class FilterDirection {
    NONE,
    HIGH_PASS_UP,       // HP 逐漸打開（經典 build）
    LOW_PASS_DOWN,      // LP 逐漸關閉
    HIGH_PASS_DOWN,     // HP 逐漸關閉（drop 後恢復）
    LOW_PASS_UP         // LP 逐漸打開
};

/**
 * 過渡事件回調
 */
struct TransitionCallbacks {
    std::function<void(int fromStyle, int toStyle)> onTransitionStart;
    std::function<void()> onTransitionComplete;
    std::function<void(int songIdx)> onSongChange;
};

/**
 * 過渡引擎 - 協調所有過渡相關邏輯
 */
class TransitionEngine {
public:
    TransitionEngine() = default;

    /**
     * 初始化
     */
    void initialize() {
        // 從 SongManager 取得第一首歌的設定
        const Song& firstSong = songManager_.getCurrentSong();
        styleMorpher_.setStyles(firstSong.styleIdx, firstSong.styleIdx);
        currentEnergy_ = firstSong.energy;
        currentVariation_ = firstSong.variation;
    }

    /**
     * 每小節開始時呼叫
     * 處理過渡邏輯和狀態更新
     */
    void notifyBarStart() {
        // 檢查 SongManager 是否需要開始過渡
        bool shouldStartTransition = songManager_.notifyBarStart();

        if (shouldStartTransition && state_ == TransitionState::IDLE) {
            startTransition();
        }

        // 更新過渡進度
        if (state_ == TransitionState::MORPHING) {
            styleMorpher_.notifyBarStart();
            updateTransitionProgress();
        }

        // 更新 filter sweep
        if (filterDirection_ != FilterDirection::NONE) {
            updateFilterSweep();
        }
    }

    /**
     * 手動觸發過渡
     */
    void triggerTransition() {
        if (state_ == TransitionState::IDLE) {
            startTransition();
        }
    }

    /**
     * 跳到指定歌曲
     */
    void jumpToSong(int songIdx) {
        // 這是立即切換，不經過過渡
        while (songManager_.getCurrentSongIdx() != songIdx) {
            songManager_.advanceToNextSong();
        }

        const Song& song = songManager_.getCurrentSong();
        styleMorpher_.setStyles(song.styleIdx, song.styleIdx);
        currentEnergy_ = song.energy;
        currentVariation_ = song.variation;

        if (callbacks_.onSongChange) {
            callbacks_.onSongChange(songIdx);
        }
    }

    // === 狀態查詢 ===

    TransitionState getState() const { return state_; }
    bool isTransitioning() const { return state_ != TransitionState::IDLE; }
    float getTransitionProgress() const { return transitionProgress_; }

    // === 當前參數（已考慮過渡） ===

    float getCurrentEnergy() const { return currentEnergy_; }
    float getCurrentVariation() const { return currentVariation_; }
    float getFilterCutoff() const { return filterCutoff_; }

    /**
     * 取得當前風格的權重（考慮過渡混合）
     */
    const float* getWeights(Role role) const {
        return styleMorpher_.getWeights(role);
    }

    float getDensityMin(Role role) const {
        return styleMorpher_.getDensityMin(role);
    }

    float getDensityMax(Role role) const {
        return styleMorpher_.getDensityMax(role);
    }

    // === 子系統存取 ===

    SongManager& getSongManager() { return songManager_; }
    const SongManager& getSongManager() const { return songManager_; }

    StyleMorpher& getStyleMorpher() { return styleMorpher_; }
    const StyleMorpher& getStyleMorpher() const { return styleMorpher_; }

    // === 設定 ===

    void setCallbacks(const TransitionCallbacks& callbacks) {
        callbacks_ = callbacks;
    }

    void setFilterSweepEnabled(bool enabled) {
        filterSweepEnabled_ = enabled;
    }

    void setFilterSweepBars(int bars) {
        filterSweepBars_ = std::max(1, bars);
    }

    void setFilterDirection(FilterDirection dir) {
        filterDirection_ = dir;
    }

private:
    SongManager songManager_;
    StyleMorpher styleMorpher_;
    TransitionCallbacks callbacks_;

    TransitionState state_ = TransitionState::IDLE;
    float transitionProgress_ = 0.0f;

    // 當前混合參數
    float currentEnergy_ = 0.5f;
    float currentVariation_ = 0.5f;
    float targetEnergy_ = 0.5f;
    float targetVariation_ = 0.5f;

    // Filter sweep
    bool filterSweepEnabled_ = true;
    FilterDirection filterDirection_ = FilterDirection::NONE;
    int filterSweepBars_ = 4;
    int filterSweepBarCount_ = 0;
    float filterCutoff_ = 1.0f;  // 0.0 = closed, 1.0 = open

    /**
     * 開始過渡
     */
    void startTransition() {
        const Song& current = songManager_.getCurrentSong();
        const Song& next = songManager_.getNextSong();

        // 設定 StyleMorpher
        styleMorpher_.setStyles(current.styleIdx, next.styleIdx);
        styleMorpher_.startTransition(songManager_.getTransitionDuration());

        // 設定目標參數
        targetEnergy_ = next.energy;
        targetVariation_ = next.variation;

        // 開始 filter sweep
        if (filterSweepEnabled_) {
            filterDirection_ = FilterDirection::HIGH_PASS_UP;
            filterSweepBarCount_ = 0;
        }

        state_ = TransitionState::MORPHING;
        transitionProgress_ = 0.0f;

        if (callbacks_.onTransitionStart) {
            callbacks_.onTransitionStart(current.styleIdx, next.styleIdx);
        }
    }

    /**
     * 更新過渡進度
     */
    void updateTransitionProgress() {
        float morphProgress = styleMorpher_.getMorphProgress();
        transitionProgress_ = morphProgress;

        // 線性插值 energy 和 variation
        float startEnergy = currentEnergy_;
        float startVariation = currentVariation_;

        // 在過渡期間逐漸改變
        currentEnergy_ = startEnergy + (targetEnergy_ - startEnergy) * morphProgress;
        currentVariation_ = startVariation + (targetVariation_ - startVariation) * morphProgress;

        // 檢查過渡是否完成
        if (!styleMorpher_.isMorphing()) {
            completeTransition();
        }
    }

    /**
     * 更新 filter sweep
     */
    void updateFilterSweep() {
        filterSweepBarCount_++;
        float progress = static_cast<float>(filterSweepBarCount_) /
                        static_cast<float>(filterSweepBars_);

        if (progress >= 1.0f) {
            // Sweep 完成
            if (filterDirection_ == FilterDirection::HIGH_PASS_UP) {
                // HP sweep 完成後，開始 HP down（恢復）
                filterDirection_ = FilterDirection::HIGH_PASS_DOWN;
                filterSweepBarCount_ = 0;
            } else {
                // 完全結束
                filterDirection_ = FilterDirection::NONE;
                filterCutoff_ = 1.0f;
            }
            return;
        }

        // 計算 filter cutoff
        switch (filterDirection_) {
            case FilterDirection::HIGH_PASS_UP:
                // 0.0 -> 0.8 (不完全關閉)
                filterCutoff_ = 1.0f - (progress * 0.8f);
                break;

            case FilterDirection::HIGH_PASS_DOWN:
                // 0.2 -> 1.0
                filterCutoff_ = 0.2f + (progress * 0.8f);
                break;

            case FilterDirection::LOW_PASS_DOWN:
                filterCutoff_ = 1.0f - (progress * 0.8f);
                break;

            case FilterDirection::LOW_PASS_UP:
                filterCutoff_ = 0.2f + (progress * 0.8f);
                break;

            default:
                filterCutoff_ = 1.0f;
        }
    }

    /**
     * 完成過渡
     */
    void completeTransition() {
        state_ = TransitionState::IDLE;
        transitionProgress_ = 1.0f;

        // 確保參數已達目標值
        currentEnergy_ = targetEnergy_;
        currentVariation_ = targetVariation_;

        if (callbacks_.onTransitionComplete) {
            callbacks_.onTransitionComplete();
        }

        if (callbacks_.onSongChange) {
            callbacks_.onSongChange(songManager_.getCurrentSongIdx());
        }
    }
};

} // namespace TechnoMachine
