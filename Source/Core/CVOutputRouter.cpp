#include "CVOutputRouter.h"
#include <cmath>

namespace TechnoMachine {

CVOutputRouter::CVOutputRouter()
{
    // Initialize all routes to disabled
    for (auto& route : routes_) {
        route.store(CHANNEL_OFF, std::memory_order_relaxed);
    }
}

void CVOutputRouter::setSampleRate(double sampleRate)
{
    sampleRate_ = sampleRate;
}

void CVOutputRouter::setRoute(int signalIdx, int outputChannel)
{
    if (signalIdx >= 0 && signalIdx < NUM_CV_SIGNALS) {
        routes_[signalIdx].store(outputChannel, std::memory_order_relaxed);
    }
}

int CVOutputRouter::getRoute(int signalIdx) const
{
    if (signalIdx >= 0 && signalIdx < NUM_CV_SIGNALS) {
        return routes_[signalIdx].load(std::memory_order_relaxed);
    }
    return CHANNEL_OFF;
}

void CVOutputRouter::noteTrigger(int voiceIdx, float velocity)
{
    if (voiceIdx < 0 || voiceIdx >= NUM_VOICES) return;

    // Set trigger gate active for 1ms
    float triggerSamples = (TRIGGER_DURATION_MS / 1000.0f) * static_cast<float>(sampleRate_);
    voiceStates_[voiceIdx].triggerSamplesRemaining.store(triggerSamples, std::memory_order_relaxed);
    voiceStates_[voiceIdx].currentVelocity.store(velocity, std::memory_order_relaxed);
}

void CVOutputRouter::setVoiceFrequency(int voiceIdx, float freq)
{
    if (voiceIdx < 0 || voiceIdx >= NUM_VOICES) return;
    voiceStates_[voiceIdx].currentFrequency.store(freq, std::memory_order_relaxed);
}

float CVOutputRouter::frequencyToPitchCV(float freq) const
{
    // 1V/Oct standard: A4 (440Hz) = 0V reference
    // Scale to 0-1 range for audio output
    // 0.0 = ~13.75 Hz (A-1), 1.0 = ~14080 Hz (A9)
    // This gives roughly 10 octaves of range

    if (freq <= 0.0f) return 0.0f;

    float octavesFromA4 = std::log2(freq / 440.0f);
    // A4 = 0.5 in our 0-1 range, with 5 octaves below and 5 above
    float pitchCV = (octavesFromA4 + 5.0f) / 10.0f;

    return std::clamp(pitchCV, 0.0f, 1.0f);
}

void CVOutputRouter::process(float* const* outputChannelData, int numOutputChannels, int numSamples)
{
    for (int voiceIdx = 0; voiceIdx < NUM_VOICES; ++voiceIdx) {
        auto& state = voiceStates_[voiceIdx];

        int triggerSignalIdx = getSignalIndex(voiceIdx, CVType::TRIGGER);
        int pitchSignalIdx = getSignalIndex(voiceIdx, CVType::PITCH);
        int velocitySignalIdx = getSignalIndex(voiceIdx, CVType::VELOCITY);

        int triggerChannel = routes_[triggerSignalIdx].load(std::memory_order_relaxed);
        int pitchChannel = routes_[pitchSignalIdx].load(std::memory_order_relaxed);
        int velocityChannel = routes_[velocitySignalIdx].load(std::memory_order_relaxed);

        float triggerRemaining = state.triggerSamplesRemaining.load(std::memory_order_relaxed);
        float velocity = state.currentVelocity.load(std::memory_order_relaxed);
        float freq = state.currentFrequency.load(std::memory_order_relaxed);
        float pitchCV = frequencyToPitchCV(freq);

        for (int sample = 0; sample < numSamples; ++sample) {
            // Trigger (gate pulse)
            if (triggerChannel >= 0 && triggerChannel < numOutputChannels) {
                float triggerValue = (triggerRemaining > 0.0f) ? 1.0f : 0.0f;
                outputChannelData[triggerChannel][sample] += triggerValue;
            }

            // Pitch CV (constant)
            if (pitchChannel >= 0 && pitchChannel < numOutputChannels) {
                outputChannelData[pitchChannel][sample] += pitchCV;
            }

            // Velocity CV (held until next trigger)
            if (velocityChannel >= 0 && velocityChannel < numOutputChannels) {
                outputChannelData[velocityChannel][sample] += velocity;
            }

            // Decrement trigger gate
            if (triggerRemaining > 0.0f) {
                triggerRemaining -= 1.0f;
            }
        }

        // Store updated trigger remaining
        state.triggerSamplesRemaining.store(triggerRemaining, std::memory_order_relaxed);
    }
}

void CVOutputRouter::setDefaultRouting(int numAvailableChannels)
{
    // Channels 0-1 are stereo audio, so CV starts at channel 2
    // Default layout (4 voices):
    // Ch 2-5: Triggers (Voice 0-3)
    // Ch 6-9: Pitch (Voice 0-3)
    // Ch 10-13: Velocity (Voice 0-3)

    for (int voice = 0; voice < NUM_VOICES; ++voice) {
        int triggerCh = 2 + voice;
        int pitchCh = 6 + voice;
        int velCh = 10 + voice;

        setRoute(getSignalIndex(voice, CVType::TRIGGER),
                 triggerCh < numAvailableChannels ? triggerCh : CHANNEL_OFF);
        setRoute(getSignalIndex(voice, CVType::PITCH),
                 pitchCh < numAvailableChannels ? pitchCh : CHANNEL_OFF);
        setRoute(getSignalIndex(voice, CVType::VELOCITY),
                 velCh < numAvailableChannels ? velCh : CHANNEL_OFF);
    }
}

juce::String CVOutputRouter::getStateAsString() const
{
    juce::StringArray parts;
    for (int i = 0; i < NUM_CV_SIGNALS; ++i) {
        parts.add(juce::String(routes_[i].load(std::memory_order_relaxed)));
    }
    return parts.joinIntoString(",");
}

void CVOutputRouter::setStateFromString(const juce::String& state)
{
    juce::StringArray parts = juce::StringArray::fromTokens(state, ",", "");
    for (int i = 0; i < std::min(parts.size(), NUM_CV_SIGNALS); ++i) {
        routes_[i].store(parts[i].getIntValue(), std::memory_order_relaxed);
    }
}

} // namespace TechnoMachine
