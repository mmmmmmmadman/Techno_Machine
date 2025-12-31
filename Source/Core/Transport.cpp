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

void Transport::advance()
{
    if (!playing_) return;

    double samplesPerBeat = clock_.getSamplesPerBeat();
    double samplesPerSixteenth = samplesPerBeat / 4.0;

    double prevPosition = samplePosition_;
    samplePosition_ += 1.0;

    int prevSixteenth = static_cast<int>(prevPosition / samplesPerSixteenth);
    int newSixteenth = static_cast<int>(samplePosition_ / samplesPerSixteenth);

    sixteenthStart_ = (newSixteenth > prevSixteenth);

    if (sixteenthStart_)
    {
        currentSixteenth_ = newSixteenth % sixteenthsPerBeat_;

        int prevBeat = prevSixteenth / sixteenthsPerBeat_;
        int newBeat = newSixteenth / sixteenthsPerBeat_;

        beatStart_ = (newBeat > prevBeat);

        if (beatStart_)
        {
            currentBeat_ = newBeat % beatsPerBar_;

            int prevBar = prevBeat / beatsPerBar_;
            int newBar = newBeat / beatsPerBar_;

            barStart_ = (newBar > prevBar);

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
