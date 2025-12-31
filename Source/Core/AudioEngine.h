#pragma once

#include <JuceHeader.h>
#include "../Synthesis/MinimalDrumSynth.h"
#include "../Sequencer/TechnoPattern.h"

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

    TechnoMachine::MinimalDrumSynth& drums() { return drums_; }
    TechnoMachine::TechnoPatternEngine& patternEngine() { return patternEngine_; }

private:
    double sampleRate_ = 48000.0;
    int samplesPerBlock_ = 256;

    TechnoMachine::MinimalDrumSynth drums_;
    TechnoMachine::TechnoPatternEngine patternEngine_;

    int lastStep_ = -1;
    int lastBar_ = -1;

    void processStep(int step);
    void applySynthModifiers();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioEngine)
};
