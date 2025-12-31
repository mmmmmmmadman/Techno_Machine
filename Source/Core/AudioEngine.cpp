#include "AudioEngine.h"
#include "Transport.h"

AudioEngine::AudioEngine()
{
}

AudioEngine::~AudioEngine()
{
}

void AudioEngine::prepare(double sampleRate, int samplesPerBlock)
{
    sampleRate_ = sampleRate;
    samplesPerBlock_ = samplesPerBlock;

    drums_.setSampleRate(static_cast<float>(sampleRate));

    // 套用 Techno 預設音色（8 聲道）
    drums_.applyTechnoPreset();

    // 設定預設音量
    using Role = TechnoMachine::Role;
    drums_.setLevel(Role::TIMELINE, 0.5f);    // Hi-Hat
    drums_.setLevel(Role::FOUNDATION, 1.0f);  // Kick
    drums_.setLevel(Role::GROOVE, 0.7f);      // Clap
    drums_.setLevel(Role::LEAD, 0.5f);        // Lead perc

    // 生成初始 patterns
    regeneratePatterns(0.1f);

    lastStep_ = -1;
    lastBar_ = -1;
}

void AudioEngine::regeneratePatterns(float variation)
{
    patternEngine_.regenerate(16, variation);

    // 套用 Synth Modifiers（Variation 影響音色）
    applySynthModifiers();
}

void AudioEngine::applySynthModifiers()
{
    const auto& mods = patternEngine_.getSynthModifiers();

    // 基於預設值套用修改
    for (int v = 0; v < TechnoMachine::NUM_VOICES; v++) {
        const auto& preset = TechnoMachine::TECHNO_PRESETS[v];

        float modifiedFreq = preset.freq * mods.freqMod[v];
        float modifiedDecay = preset.decay * mods.decayMod[v];

        drums_.setVoiceParams(v, preset.mode, modifiedFreq, modifiedDecay);
    }
}

void AudioEngine::setFillInterval(int bars)
{
    patternEngine_.setFillInterval(bars);
}

int AudioEngine::getFillInterval() const
{
    return patternEngine_.getFillInterval();
}

bool AudioEngine::isFillActive() const
{
    return patternEngine_.isFillActive();
}

void AudioEngine::processStep(int step)
{
    // 使用 getActivePattern 支援 Fill
    for (int v = 0; v < TechnoMachine::NUM_VOICES; v++) {
        const auto& pattern = patternEngine_.getActivePattern(v);
        if (pattern.hasOnset(step)) {
            drums_.triggerVoice(v, pattern.getVelocity(step));
        }
    }

    // 推進 Fill 計數器
    patternEngine_.advanceStep();
}

AudioEngine::StereoOutput AudioEngine::process(const Transport& transport)
{
    // Check for new bar (Fill 觸發)
    int currentBar = transport.getCurrentBar();
    if (transport.isBarStart() && currentBar != lastBar_) {
        patternEngine_.notifyBarStart(currentBar);
        lastBar_ = currentBar;
    }

    // Check for new step
    int currentStep = transport.getCurrentSixteenth();

    if (transport.isSixteenthStart() && currentStep != lastStep_) {
        processStep(currentStep);
        lastStep_ = currentStep;
    }

    // Process all drum voices and return stereo output
    auto output = drums_.process();
    return { output.left, output.right };
}
