#pragma once

#include <JuceHeader.h>
#include "Clock.h"

class Transport
{
public:
    Transport();
    ~Transport();

    void prepare(double sampleRate);
    void setTempo(double bpm);
    double getTempo() const { return clock_.getTempo(); }

    void start();
    void stop();
    void reset();

    bool isPlaying() const { return playing_; }
    void advance();

    int getCurrentBar() const { return currentBar_; }
    int getCurrentBeat() const { return currentBeat_; }
    int getCurrentSixteenth() const { return currentSixteenth_; }

    bool isBeatStart() const { return beatStart_; }
    bool isBarStart() const { return barStart_; }
    bool isSixteenthStart() const { return sixteenthStart_; }

    double getPositionInBar() const;

private:
    Clock clock_;
    bool playing_ = false;

    int currentBar_ = 0;
    int currentBeat_ = 0;
    int currentSixteenth_ = 0;

    double samplePosition_ = 0.0;
    double lastBeatPosition_ = 0.0;

    bool beatStart_ = false;
    bool barStart_ = false;
    bool sixteenthStart_ = false;

    static constexpr int beatsPerBar_ = 4;
    static constexpr int sixteenthsPerBeat_ = 4;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Transport)
};
