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

    // 初始化 TransitionEngine
    transitionEngine_.initialize();

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

// === 風格控制 ===

void AudioEngine::setStyle(int styleIdx)
{
    patternEngine_.setStyle(styleIdx);
    regeneratePatterns(transitionEngine_.getCurrentVariation());
}

void AudioEngine::setStyle(TechnoMachine::StyleType style)
{
    setStyle(static_cast<int>(style));
}

int AudioEngine::getStyleIdx() const
{
    return patternEngine_.getStyleIdx();
}

const char* AudioEngine::getStyleName() const
{
    return patternEngine_.getStyleName();
}

// === DJ Set 控制 ===

void AudioEngine::generateRandomSet(int numSongs)
{
    transitionEngine_.getSongManager().generateRandomSet(numSongs);
    transitionEngine_.initialize();

    // 套用第一首歌的設定
    const auto& song = transitionEngine_.getSongManager().getCurrentSong();
    patternEngine_.setStyle(song.styleIdx);
    regeneratePatterns(song.variation);
}

void AudioEngine::triggerNextSong()
{
    transitionEngine_.triggerTransition();
}

void AudioEngine::jumpToSong(int songIdx)
{
    transitionEngine_.jumpToSong(songIdx);

    // 套用新歌的設定
    const auto& song = transitionEngine_.getSongManager().getCurrentSong();
    patternEngine_.setStyle(song.styleIdx);
    regeneratePatterns(song.variation);
}

bool AudioEngine::isTransitioning() const
{
    return transitionEngine_.isTransitioning();
}

float AudioEngine::getTransitionProgress() const
{
    return transitionEngine_.getTransitionProgress();
}

void AudioEngine::applyTransitionParameters()
{
    // 過渡期間，使用 TransitionEngine 提供的混合權重
    if (transitionEngine_.isTransitioning()) {
        // 更新 pattern engine 使用混合後的權重
        // 這裡可以根據需要實作更精細的控制
    }

    // 套用 filter cutoff（用於 HP/LP sweep）
    // 這裡可以連接到實際的 filter 參數
    // float cutoff = transitionEngine_.getFilterCutoff();
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
    // Check for new bar (Fill 觸發 + TransitionEngine 更新)
    int currentBar = transport.getCurrentBar();
    if (transport.isBarStart() && currentBar != lastBar_) {
        patternEngine_.notifyBarStart(currentBar);
        transitionEngine_.notifyBarStart();

        // 如果過渡中，套用混合參數
        if (transitionEngine_.isTransitioning()) {
            applyTransitionParameters();
        }

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
