#pragma once

#include <JuceHeader.h>

class Clock
{
public:
    Clock();
    ~Clock();

    void prepare(double sampleRate);
    void setTempo(double bpm);
    double getTempo() const { return tempo_; }

    void reset();
    void advance();

    double getPhase() const { return phase_; }
    double getSamplesPerBeat() const { return samplesPerBeat_; }

    bool isBeatPulse() const { return beatPulse_; }

private:
    double sampleRate_ = 48000.0;
    double tempo_ = 128.0;
    double phase_ = 0.0;
    double samplesPerBeat_ = 0.0;
    bool beatPulse_ = false;

    void updateSamplesPerBeat();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Clock)
};
