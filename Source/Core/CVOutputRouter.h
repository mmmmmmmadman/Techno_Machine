#pragma once

#include <JuceHeader.h>
#include <array>
#include <atomic>
#include <functional>
#include "../Synthesis/MinimalDrumSynth.h"  // for NUM_VOICES

namespace TechnoMachine {

static constexpr int NUM_CV_TYPES = 3;  // Trigger, Pitch, Velocity
static constexpr int NUM_CV_SIGNALS = NUM_VOICES * NUM_CV_TYPES;  // 24

enum class CVType {
    TRIGGER = 0,
    PITCH = 1,
    VELOCITY = 2
};

class CVOutputRouter {
public:
    static constexpr float TRIGGER_DURATION_MS = 1.0f;
    static constexpr int CHANNEL_OFF = -1;

    CVOutputRouter();

    // Configuration
    void setSampleRate(double sampleRate);
    double getSampleRate() const { return sampleRate_; }

    // Routing: signalIdx = voiceIdx * 3 + cvType
    void setRoute(int signalIdx, int outputChannel);
    int getRoute(int signalIdx) const;

    // Helper to get signal index
    static int getSignalIndex(int voiceIdx, CVType type) {
        return voiceIdx * NUM_CV_TYPES + static_cast<int>(type);
    }

    // Called when a voice triggers
    void noteTrigger(int voiceIdx, float velocity);

    // Called when voice frequency changes
    void setVoiceFrequency(int voiceIdx, float freq);

    // Process and write CV to output buffer (additive)
    void process(float* const* outputChannelData, int numOutputChannels, int numSamples);

    // Default routing for N available channels
    void setDefaultRouting(int numAvailableChannels);

    // Serialization
    juce::String getStateAsString() const;
    void setStateFromString(const juce::String& state);

private:
    double sampleRate_ = 48000.0;

    // Routes: outputChannel for each signal (-1 = disabled)
    std::array<std::atomic<int>, NUM_CV_SIGNALS> routes_;

    // Per-voice state
    struct VoiceState {
        std::atomic<float> triggerSamplesRemaining{0.0f};
        std::atomic<float> currentVelocity{0.0f};
        std::atomic<float> currentFrequency{440.0f};
    };
    std::array<VoiceState, NUM_VOICES> voiceStates_;

    // Convert frequency to pitch CV (0-1 range, 1V/Oct equivalent)
    float frequencyToPitchCV(float freq) const;
};

} // namespace TechnoMachine
