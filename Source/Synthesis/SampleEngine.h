/**
 * SampleEngine.h
 * Techno Machine - Sample playback engine
 *
 * Allows loading one-shot samples per role (4 slots)
 * Works alongside MinimalDrumSynth for hybrid synth+sample sounds
 */

#pragma once

#include <JuceHeader.h>
#include <atomic>

namespace TechnoMachine {

/**
 * Single sample voice - loads and plays one WAV/AIFF file
 */
class SampleVoice {
public:
    SampleVoice() = default;

    /**
     * Load sample from file
     * @param file WAV or AIFF file
     * @param targetSampleRate Resample to this rate if different
     * @return true if loaded successfully
     */
    bool loadFromFile(const juce::File& file, double targetSampleRate) {
        juce::AudioFormatManager formatManager;
        formatManager.registerBasicFormats();

        std::unique_ptr<juce::AudioFormatReader> reader(
            formatManager.createReaderFor(file));

        if (!reader) {
            return false;
        }

        // Read into temporary buffer
        juce::AudioBuffer<float> tempBuffer;
        tempBuffer.setSize(1, static_cast<int>(reader->lengthInSamples));
        reader->read(&tempBuffer, 0, tempBuffer.getNumSamples(), 0, true, false);

        // Resample if needed
        if (std::abs(reader->sampleRate - targetSampleRate) > 1.0) {
            double ratio = targetSampleRate / reader->sampleRate;
            int newLength = static_cast<int>(tempBuffer.getNumSamples() * ratio);

            juce::AudioBuffer<float> resampledBuffer;
            resampledBuffer.setSize(1, newLength);

            // Simple linear interpolation resampling
            for (int i = 0; i < newLength; ++i) {
                double srcPos = i / ratio;
                int srcIdx = static_cast<int>(srcPos);
                float frac = static_cast<float>(srcPos - srcIdx);

                if (srcIdx + 1 < tempBuffer.getNumSamples()) {
                    float s0 = tempBuffer.getSample(0, srcIdx);
                    float s1 = tempBuffer.getSample(0, srcIdx + 1);
                    resampledBuffer.setSample(0, i, s0 + frac * (s1 - s0));
                } else if (srcIdx < tempBuffer.getNumSamples()) {
                    resampledBuffer.setSample(0, i, tempBuffer.getSample(0, srcIdx));
                }
            }

            buffer_ = std::move(resampledBuffer);
        } else {
            buffer_ = std::move(tempBuffer);
        }

        fileName_ = file.getFileName();
        filePath_ = file.getFullPathName();
        loaded_ = true;
        return true;
    }

    /**
     * Trigger sample playback
     */
    void trigger(float velocity) {
        velocity_ = velocity;
        playbackPosition_ = 0;
        isPlaying_ = true;
    }

    /**
     * Process one sample
     * @return Mono output sample
     */
    float process() {
        if (!isPlaying_ || !loaded_ || buffer_.getNumSamples() == 0) {
            return 0.0f;
        }

        if (playbackPosition_ >= buffer_.getNumSamples()) {
            isPlaying_ = false;
            return 0.0f;
        }

        float sample = buffer_.getSample(0, playbackPosition_) * velocity_;
        playbackPosition_++;

        return sample;
    }

    bool isLoaded() const { return loaded_; }
    bool isPlaying() const { return isPlaying_; }
    juce::String getFileName() const { return fileName_; }
    juce::String getFilePath() const { return filePath_; }

    void clear() {
        buffer_.setSize(0, 0);
        loaded_ = false;
        isPlaying_ = false;
        fileName_ = "";
        filePath_ = "";
    }

private:
    juce::AudioBuffer<float> buffer_;
    int playbackPosition_ = 0;
    bool isPlaying_ = false;
    bool loaded_ = false;
    float velocity_ = 0.0f;
    juce::String fileName_;
    juce::String filePath_;
};

/**
 * Stereo output structure
 */
struct SampleStereoOutput {
    float left = 0.0f;
    float right = 0.0f;
};

/**
 * Sample engine - manages 4 sample slots (1 per role)
 */
class SampleEngine {
public:
    static constexpr int NUM_ROLES = 4;
    static constexpr int NUM_VOICES = 4;  // 1 per role

    SampleEngine() = default;

    /**
     * Prepare engine with sample rate
     */
    void prepare(double sampleRate) {
        sampleRate_ = sampleRate;
    }

    /**
     * Load sample for a specific voice
     * @param voiceIdx 0-3 (one per role)
     * @param file WAV or AIFF file
     * @return true if loaded successfully
     */
    bool loadSample(int voiceIdx, const juce::File& file) {
        if (voiceIdx < 0 || voiceIdx >= NUM_VOICES) {
            return false;
        }
        return samples_[voiceIdx].loadFromFile(file, sampleRate_);
    }

    /**
     * Clear sample for a specific voice
     */
    void clearSample(int voiceIdx) {
        if (voiceIdx >= 0 && voiceIdx < NUM_VOICES) {
            samples_[voiceIdx].clear();
        }
    }

    /**
     * Trigger sample for a voice
     */
    void triggerVoice(int voiceIdx, float velocity) {
        if (voiceIdx >= 0 && voiceIdx < NUM_VOICES && samples_[voiceIdx].isLoaded()) {
            samples_[voiceIdx].trigger(velocity);
        }
    }

    /**
     * Process all samples and return stereo mix
     */
    SampleStereoOutput process() {
        SampleStereoOutput output;

        // Voice panning (same as synth voices)
        // Timeline (0)=L, Foundation (1)=C, Groove (2)=C, Lead (3)=R
        static const float panL[NUM_VOICES] = {0.7f, 0.5f, 0.5f, 0.3f};
        static const float panR[NUM_VOICES] = {0.3f, 0.5f, 0.5f, 0.7f};

        for (int v = 0; v < NUM_VOICES; ++v) {
            float sample = samples_[v].process() * roleLevel_[v];
            output.left += sample * panL[v];
            output.right += sample * panR[v];
        }

        return output;
    }

    /**
     * Check if voice has a sample loaded
     */
    bool hasSample(int voiceIdx) const {
        if (voiceIdx < 0 || voiceIdx >= NUM_VOICES) return false;
        return samples_[voiceIdx].isLoaded();
    }

    /**
     * Get sample filename for a voice
     */
    juce::String getSampleName(int voiceIdx) const {
        if (voiceIdx < 0 || voiceIdx >= NUM_VOICES) return "";
        return samples_[voiceIdx].getFileName();
    }

    /**
     * Get full sample path for a voice
     */
    juce::String getSamplePath(int voiceIdx) const {
        if (voiceIdx < 0 || voiceIdx >= NUM_VOICES) return "";
        return samples_[voiceIdx].getFilePath();
    }

    /**
     * Set level for a role
     */
    void setRoleLevel(int roleIdx, float level) {
        if (roleIdx >= 0 && roleIdx < NUM_ROLES) {
            roleLevel_[roleIdx] = std::clamp(level, 0.0f, 2.0f);
        }
    }

private:
    SampleVoice samples_[NUM_VOICES];
    double sampleRate_ = 48000.0;
    float roleLevel_[NUM_ROLES] = {1.0f, 1.0f, 1.0f, 1.0f};
};

} // namespace TechnoMachine
