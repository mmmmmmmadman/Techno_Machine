/**
 * SongManager.hpp
 * Techno Machine - 歌曲序列管理
 *
 * 管理 DJ Set 中的歌曲序列和切換邏輯
 */

#pragma once

#include "../Sequencer/StyleProfiles.hpp"
#include "StyleMorpher.hpp"
#include <vector>
#include <random>
#include <algorithm>

namespace TechnoMachine {

/**
 * 複合風格 - 每個角色可以有不同的風格
 */
struct CompositeStyle {
    int roleStyles[NUM_ROLES];  // 每個角色的風格索引

    CompositeStyle() {
        for (int i = 0; i < NUM_ROLES; i++) {
            roleStyles[i] = 0;  // 預設 Techno
        }
    }

    CompositeStyle(int uniformStyle) {
        for (int i = 0; i < NUM_ROLES; i++) {
            roleStyles[i] = uniformStyle;
        }
    }

    int getStyle(Role role) const {
        if (role >= 0 && role < NUM_ROLES) {
            return roleStyles[role];
        }
        return 0;
    }

    void setStyle(Role role, int styleIdx) {
        if (role >= 0 && role < NUM_ROLES) {
            roleStyles[role] = std::clamp(styleIdx, 0, NUM_STYLES - 1);
        }
    }

    // 取得主要風格（用於顯示）- 取出現最多次的風格
    int getDominantStyle() const {
        int counts[NUM_STYLES] = {0};
        for (int i = 0; i < NUM_ROLES; i++) {
            counts[roleStyles[i]]++;
        }
        int maxIdx = 0;
        for (int i = 1; i < NUM_STYLES; i++) {
            if (counts[i] > counts[maxIdx]) maxIdx = i;
        }
        return maxIdx;
    }
};

/**
 * 單首歌曲定義
 */
struct Song {
    CompositeStyle compositeStyle;  // 複合風格（per-role）
    float variation;                // Variation 值 (0.0 - 1.0)
    int durationBars;               // 持續小節數
    float energy;                   // 能量等級 (0.0 - 1.0)

    // 保留舊的 styleIdx 介面以便相容
    int styleIdx;

    Song(int style = 0, float var = 0.5f, int bars = 64, float e = 0.5f)
        : compositeStyle(style), variation(var), durationBars(bars), energy(e), styleIdx(style) {}

    Song(const CompositeStyle& cs, float var = 0.5f, int bars = 64, float e = 0.5f)
        : compositeStyle(cs), variation(var), durationBars(bars), energy(e),
          styleIdx(cs.getDominantStyle()) {}
};

/**
 * 換歌觸發模式
 */
enum class TransitionTrigger {
    FIXED_BARS,     // 固定小節數後換歌
    MANUAL,         // 手動觸發
    AUTO_ENERGY     // 根據能量曲線自動換歌
};

/**
 * 歌曲序列管理器
 */
class SongManager {
public:
    SongManager() : rng_(std::random_device{}()) {
        // 預設產生一個隨機 set
        generateRandomSet(8);
    }

    /**
     * 產生隨機 Set（使用複合風格）
     * @param numSongs 歌曲數量
     * @param fixedBars 固定小節數（0 表示隨機 32-128）
     */
    void generateRandomSet(int numSongs, int fixedBars = 0) {
        songs_.clear();

        std::uniform_int_distribution<int> styleDist(0, NUM_STYLES - 1);
        std::uniform_real_distribution<float> varDist(0.2f, 0.8f);
        std::uniform_int_distribution<int> barsDist(32, 128);
        std::uniform_real_distribution<float> energyDist(0.3f, 0.9f);
        std::uniform_int_distribution<int> roleDist(0, NUM_ROLES - 1);
        std::uniform_real_distribution<float> probDist(0.0f, 1.0f);

        CompositeStyle prevStyle;

        for (int i = 0; i < numSongs; i++) {
            int bars = (fixedBars > 0) ? fixedBars : barsDist(rng_);

            CompositeStyle newStyle;

            if (i == 0) {
                // 第一首歌：每個角色隨機選擇風格
                for (int r = 0; r < NUM_ROLES; r++) {
                    newStyle.roleStyles[r] = styleDist(rng_);
                }
            } else {
                // 後續歌曲：根據連續性邏輯生成
                newStyle = generateContinuousStyle(prevStyle, styleDist, probDist);
            }

            songs_.emplace_back(
                newStyle,
                varDist(rng_),
                bars,
                energyDist(rng_)
            );

            prevStyle = newStyle;
        }

        currentSongIdx_ = 0;
        barsInCurrentSong_ = 0;
    }

    /**
     * 生成與前一首有連續性但明顯變化的複合風格
     * 規則：
     * - 保持 1-2 個角色不變（連續性）
     * - 至少 2 個角色必須有「明顯差異」的風格變化（dissimilarity >= 0.5）
     */
    CompositeStyle generateContinuousStyle(
        const CompositeStyle& prev,
        std::uniform_int_distribution<int>& styleDist,
        std::uniform_real_distribution<float>& probDist)
    {
        CompositeStyle newStyle;
        const float MIN_DISSIMILARITY = 0.5f;  // 明顯差異的門檻

        // 決定要保持幾個角色不變（1-2 個）
        int keepCount = (probDist(rng_) < 0.5f) ? 1 : 2;

        // 隨機選擇要保持的角色
        std::vector<int> roles = {0, 1, 2, 3};
        std::shuffle(roles.begin(), roles.end(), rng_);

        int bigChangeCount = 0;  // 追蹤明顯變化的角色數

        for (int i = 0; i < NUM_ROLES; i++) {
            int role = roles[static_cast<size_t>(i)];
            int prevStyleIdx = prev.roleStyles[role];

            if (i < keepCount) {
                // 保持不變
                newStyle.roleStyles[role] = prevStyleIdx;
            } else {
                // 需要變化的角色
                int newStyleIdx;

                // 如果還沒有達到 2 個明顯變化，強制選擇差異大的風格
                if (bigChangeCount < 2) {
                    // 找出差異度 >= 0.5 的風格
                    int dissimilarStyles[NUM_STYLES];
                    int dissimilarCount = findDissimilarStyles(prevStyleIdx, MIN_DISSIMILARITY, dissimilarStyles);

                    if (dissimilarCount > 0) {
                        // 從差異大的風格中隨機選擇
                        std::uniform_int_distribution<int> dissimilarDist(0, dissimilarCount - 1);
                        newStyleIdx = dissimilarStyles[dissimilarDist(rng_)];
                        bigChangeCount++;
                    } else {
                        // 找不到差異大的，退回一般隨機
                        newStyleIdx = styleDist(rng_);
                        if (getStyleDissimilarity(prevStyleIdx, newStyleIdx) >= MIN_DISSIMILARITY) {
                            bigChangeCount++;
                        }
                    }
                } else {
                    // 已達到 2 個明顯變化，可以選擇任意風格
                    newStyleIdx = styleDist(rng_);
                }

                newStyle.roleStyles[role] = newStyleIdx;
            }
        }

        return newStyle;
    }

    /**
     * 設定所有歌曲的長度
     */
    void setAllSongDuration(int bars) {
        for (auto& song : songs_) {
            song.durationBars = std::max(8, bars);
        }
    }

    /**
     * 新增歌曲到 Set
     */
    void addSong(const Song& song) {
        songs_.push_back(song);
    }

    void addSong(int style, float variation, int bars, float energy) {
        songs_.emplace_back(style, variation, bars, energy);
    }

    /**
     * 清除 Set
     */
    void clear() {
        songs_.clear();
        currentSongIdx_ = 0;
        barsInCurrentSong_ = 0;
    }

    /**
     * 取得當前歌曲
     */
    const Song& getCurrentSong() const {
        if (songs_.empty()) {
            static Song defaultSong;
            return defaultSong;
        }
        return songs_[currentSongIdx_ % songs_.size()];
    }

    /**
     * 取得下一首歌曲
     */
    const Song& getNextSong() const {
        if (songs_.empty()) {
            static Song defaultSong;
            return defaultSong;
        }
        size_t nextIdx = (currentSongIdx_ + 1) % songs_.size();
        return songs_[nextIdx];
    }

    /**
     * 通知小節開始
     * @return true 如果需要開始換歌過渡
     */
    bool notifyBarStart() {
        barsInCurrentSong_++;

        if (triggerMode_ == TransitionTrigger::FIXED_BARS) {
            const Song& current = getCurrentSong();

            // 計算 phrase-aligned 過渡開始點
            int transitionStart = calculatePhraseAlignedTransitionStart(current.durationBars);

            if (barsInCurrentSong_ == transitionStart) {
                return true;  // 需要開始過渡
            }

            if (barsInCurrentSong_ >= current.durationBars) {
                // 切換到下一首
                advanceToNextSong();
            }
        }

        return false;
    }

    /**
     * 手動觸發換歌
     */
    void triggerNextSong() {
        if (triggerMode_ == TransitionTrigger::MANUAL) {
            // 直接開始過渡
            advanceToNextSong();
        }
    }

    /**
     * 前進到下一首歌
     */
    void advanceToNextSong() {
        if (!songs_.empty()) {
            currentSongIdx_ = (currentSongIdx_ + 1) % songs_.size();
            barsInCurrentSong_ = 0;
        }
    }

    // 設定和狀態
    void setTriggerMode(TransitionTrigger mode) { triggerMode_ = mode; }
    TransitionTrigger getTriggerMode() const { return triggerMode_; }

    void setTransitionDuration(int bars) { transitionDurationBars_ = std::max(1, bars); }
    int getTransitionDuration() const { return transitionDurationBars_; }

    void setPhraseLength(int bars) { phraseLength_ = std::max(4, bars); }
    int getPhraseLength() const { return phraseLength_; }

    int getCurrentSongIdx() const { return currentSongIdx_; }
    int getBarsInCurrentSong() const { return barsInCurrentSong_; }
    int getSongCount() const { return static_cast<int>(songs_.size()); }

    float getProgress() const {
        if (songs_.empty()) return 0.0f;
        const Song& current = getCurrentSong();
        return static_cast<float>(barsInCurrentSong_) / static_cast<float>(current.durationBars);
    }

private:
    std::vector<Song> songs_;
    int currentSongIdx_ = 0;
    int barsInCurrentSong_ = 0;

    TransitionTrigger triggerMode_ = TransitionTrigger::FIXED_BARS;
    int transitionDurationBars_ = 8;  // 過渡持續小節數
    int phraseLength_ = 8;  // 標準 phrase 長度（8 bars）

    std::mt19937 rng_;

    /**
     * 計算 phrase-aligned 過渡開始點
     * 確保過渡在 phrase 邊界開始
     */
    int calculatePhraseAlignedTransitionStart(int songDuration) const {
        // 基本過渡開始點
        int basicStart = songDuration - transitionDurationBars_;

        // 找到最接近的 phrase 邊界（向下取整到 phrase 倍數）
        int alignedStart = (basicStart / phraseLength_) * phraseLength_;

        // 確保至少有過渡時間
        if (songDuration - alignedStart < transitionDurationBars_) {
            // 提前一個 phrase
            alignedStart = std::max(phraseLength_, alignedStart - phraseLength_);
        }

        return alignedStart;
    }
};

} // namespace TechnoMachine
