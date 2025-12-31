#pragma once

#include <JuceHeader.h>
#include <random>
#include "../Synthesis/MinimalDrumSynth.h"
#include "../Sequencer/TechnoPattern.h"
#include "../Arrangement/TransitionEngine.hpp"

class Transport;

class AudioEngine
{
public:
    struct StereoOutput {
        float left;
        float right;
    };

    AudioEngine();
    ~AudioEngine();

    void prepare(double sampleRate, int samplesPerBlock);
    StereoOutput process(const Transport& transport);

    void regeneratePatterns(float variation = 0.1f);

    // Fill 控制
    void setFillInterval(int bars);
    int getFillInterval() const;
    bool isFillActive() const;

    // Density 控制（生成時使用）
    void setDensity(TechnoMachine::Role role, float density);
    float getDensity(TechnoMachine::Role role) const;

    // Playback Density（即時過濾，不重新生成）
    void setPlaybackDensity(TechnoMachine::Role role, float density);
    float getPlaybackDensity(TechnoMachine::Role role) const;

    // 風格控制
    void setStyle(int styleIdx);
    void setStyle(TechnoMachine::StyleType style);
    int getStyleIdx() const;
    const char* getStyleName() const;

    // DJ Set 控制
    void generateRandomSet(int numSongs, int barsPerSong = 0);
    void triggerNextSong();
    void jumpToSong(int songIdx);
    bool isTransitioning() const;
    float getTransitionProgress() const;

    // DJ Set 設定
    void setSongDuration(int bars);
    void setTransitionDuration(int bars);
    int getTransitionDuration() const;

    // === 手動 Crossfader 控制 ===
    void setCrossfader(float position);  // 0.0 = Deck A, 1.0 = Deck B
    float getCrossfader() const;
    void loadNextSong();                 // 載入下一首到非作用中的 Deck
    void loadToDeck(int deck);           // 載入隨機歌曲到指定 Deck (0=A, 1=B)
    const char* getDeckAStyleName() const;
    const char* getDeckBStyleName() const;
    const char* getDeckRoleStyleName(int deck, TechnoMachine::Role role) const;

    // 子系統存取
    TechnoMachine::MinimalDrumSynth& drums() { return drums_; }
    TechnoMachine::TechnoPatternEngine& patternEngine() { return patternEngine_; }
    TechnoMachine::TransitionEngine& transitionEngine() { return transitionEngine_; }

private:
    double sampleRate_ = 48000.0;
    int samplesPerBlock_ = 256;

    TechnoMachine::MinimalDrumSynth drums_;
    TechnoMachine::TechnoPatternEngine patternEngine_;
    TechnoMachine::TransitionEngine transitionEngine_;

    int lastStep_ = -1;
    int lastBar_ = -1;

    // Playback density per role（1.0 = 全部播放，0.0 = 靜音）
    float playbackDensity_[TechnoMachine::NUM_ROLES] = {1.0f, 1.0f, 1.0f, 1.0f};

    // 用於 density 過濾的隨機數生成器
    std::mt19937 densityRng_{std::random_device{}()};

    void processStep(int step);
    void applySynthModifiers();
    void applyTransitionParameters();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioEngine)
};
