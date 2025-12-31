#include "Transport.h"

Transport::Transport()
{
}

Transport::~Transport()
{
}

void Transport::prepare(double sampleRate)
{
    clock_.prepare(sampleRate);
    reset();
}

void Transport::setTempo(double bpm)
{
    clock_.setTempo(bpm);
}

void Transport::start()
{
    if (!playing_)
    {
        playing_ = true;
    }
}

void Transport::stop()
{
    playing_ = false;
}

void Transport::reset()
{
    clock_.reset();
    currentBar_ = 0;
    currentBeat_ = 0;
    currentSixteenth_ = 0;
    samplePosition_ = 0.0;
    lastBeatPosition_ = 0.0;
    beatStart_ = false;
    barStart_ = false;
    sixteenthStart_ = false;
}

void Transport::setSwingLevel(int level)
{
    swingLevel_ = std::clamp(level, 0, 3);
}

void Transport::advance()
{
    if (!playing_) return;

    double samplesPerBeat = clock_.getSamplesPerBeat();
    double samplesPerEighth = samplesPerBeat / 2.0;

    double prevPosition = samplePosition_;
    samplePosition_ += 1.0;

    // Calculate swing-adjusted 16th note positions
    // Swing delays the off-beat 16ths (positions 1 and 3 within each beat)
    // Each 8th note pair has: on-beat at 0%, off-beat at swingAmount%
    float swingRatio = swingAmounts_[swingLevel_];

    // Position within current 8th note pair (0.0 to 1.0 covers two 16ths)
    double posInEighth = std::fmod(samplePosition_, samplesPerEighth);
    double prevPosInEighth = std::fmod(prevPosition, samplesPerEighth);

    // Threshold for the off-beat 16th within each 8th note
    double swingThreshold = samplesPerEighth * swingRatio;

    // Detect 16th note crossings with swing adjustment
    int eighthIdx = static_cast<int>(samplePosition_ / samplesPerEighth);
    int prevEighthIdx = static_cast<int>(prevPosition / samplesPerEighth);

    bool onBeatCrossed = (eighthIdx > prevEighthIdx);
    bool offBeatCrossed = (posInEighth >= swingThreshold && prevPosInEighth < swingThreshold);

    sixteenthStart_ = onBeatCrossed || offBeatCrossed;

    if (sixteenthStart_)
    {
        // Calculate which 16th we're on (0-15 in a bar)
        int globalSixteenth;
        if (onBeatCrossed) {
            // On-beat 16th (even: 0, 2, 4, 6, 8, 10, 12, 14)
            globalSixteenth = eighthIdx * 2;
        } else {
            // Off-beat 16th (odd: 1, 3, 5, 7, 9, 11, 13, 15)
            globalSixteenth = eighthIdx * 2 + 1;
        }

        currentSixteenth_ = globalSixteenth % sixteenthsPerBeat_;

        int newBeat = globalSixteenth / sixteenthsPerBeat_;
        int prevBeat = (globalSixteenth - 1) / sixteenthsPerBeat_;

        beatStart_ = (currentSixteenth_ == 0) && onBeatCrossed;

        if (beatStart_)
        {
            currentBeat_ = newBeat % beatsPerBar_;

            int newBar = newBeat / beatsPerBar_;
            int prevBar = (newBeat - 1) / beatsPerBar_;

            barStart_ = (currentBeat_ == 0);

            if (barStart_)
            {
                currentBar_ = newBar;
            }
        }
        else
        {
            barStart_ = false;
        }
    }
    else
    {
        beatStart_ = false;
        barStart_ = false;
    }

    clock_.advance();
}

double Transport::getPositionInBar() const
{
    double samplesPerBar = clock_.getSamplesPerBeat() * beatsPerBar_;
    return std::fmod(samplePosition_, samplesPerBar) / samplesPerBar;
}
