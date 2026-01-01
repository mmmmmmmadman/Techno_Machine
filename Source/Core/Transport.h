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

    // Swing control (0=off, 1=light, 2=medium, 3=heavy)
    void setSwingLevel(int level);
    int getSwingLevel() const { return swingLevel_; }

    // Set swing by ratio (0.5 = straight, 0.67 = triplet)
    // Maps to nearest level automatically
    void setSwingRatio(float ratio);
    float getSwingRatio() const { return swingAmounts_[swingLevel_]; }

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

    // Swing: delays off-beat 16th notes
    // Level 0: 50% (straight), 1: 54%, 2: 62%, 3: 67% (triplet)
    int swingLevel_ = 0;
    static constexpr float swingAmounts_[4] = {0.50f, 0.54f, 0.62f, 0.67f};

    static constexpr int beatsPerBar_ = 4;
    static constexpr int sixteenthsPerBeat_ = 4;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Transport)
};
