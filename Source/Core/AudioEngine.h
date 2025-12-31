#pragma once

#include <JuceHeader.h>
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

    // 風格控制
    void setStyle(int styleIdx);
    void setStyle(TechnoMachine::StyleType style);
    int getStyleIdx() const;
    const char* getStyleName() const;

    // DJ Set 控制
    void generateRandomSet(int numSongs);
    void triggerNextSong();
    void jumpToSong(int songIdx);
    bool isTransitioning() const;
    float getTransitionProgress() const;

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

    void processStep(int step);
    void applySynthModifiers();
    void applyTransitionParameters();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioEngine)
};
