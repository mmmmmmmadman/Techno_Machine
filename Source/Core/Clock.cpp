#include "Clock.h"

Clock::Clock()
{
    updateSamplesPerBeat();
}

Clock::~Clock()
{
}

void Clock::prepare(double sampleRate)
{
    sampleRate_ = sampleRate;
    updateSamplesPerBeat();
    reset();
}

void Clock::setTempo(double bpm)
{
    tempo_ = juce::jlimit(20.0, 300.0, bpm);
    updateSamplesPerBeat();
}

void Clock::reset()
{
    phase_ = 0.0;
    beatPulse_ = false;
}

void Clock::advance()
{
    double prevPhase = phase_;
    phase_ += 1.0 / samplesPerBeat_;

    beatPulse_ = (static_cast<int>(phase_) > static_cast<int>(prevPhase));

    if (phase_ >= 1.0)
    {
        phase_ -= 1.0;
    }
}

void Clock::updateSamplesPerBeat()
{
    samplesPerBeat_ = (60.0 / tempo_) * sampleRate_;
}
