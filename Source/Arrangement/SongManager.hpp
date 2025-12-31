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

namespace TechnoMachine {

/**
 * 單首歌曲定義
 */
struct Song {
    int styleIdx;           // 風格索引
    float variation;        // Variation 值 (0.0 - 1.0)
    int durationBars;       // 持續小節數
    float energy;           // 能量等級 (0.0 - 1.0)

    Song(int style = 0, float var = 0.5f, int bars = 64, float e = 0.5f)
        : styleIdx(style), variation(var), durationBars(bars), energy(e) {}
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
     * 產生隨機 Set
     * @param numSongs 歌曲數量
     * @param fixedBars 固定小節數（0 表示隨機 32-128）
     */
    void generateRandomSet(int numSongs, int fixedBars = 0) {
        songs_.clear();

        std::uniform_int_distribution<int> styleDist(0, NUM_STYLES - 1);
        std::uniform_real_distribution<float> varDist(0.2f, 0.8f);
        std::uniform_int_distribution<int> barsDist(32, 128);
        std::uniform_real_distribution<float> energyDist(0.3f, 0.9f);

        for (int i = 0; i < numSongs; i++) {
            int bars = (fixedBars > 0) ? fixedBars : barsDist(rng_);
            songs_.emplace_back(
                styleDist(rng_),
                varDist(rng_),
                bars,
                energyDist(rng_)
            );
        }

        currentSongIdx_ = 0;
        barsInCurrentSong_ = 0;
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
