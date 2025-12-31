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

    // 設定過渡回調
    TechnoMachine::TransitionCallbacks callbacks;
    callbacks.onTransitionStart = [this](int fromStyle, int toStyle) {
        // 過渡開始時：切換風格並啟動 crossfade
        patternEngine_.setStyle(toStyle);
        const auto& nextSong = transitionEngine_.getSongManager().getNextSong();
        int transitionBars = transitionEngine_.getSongManager().getTransitionDuration();
        patternEngine_.startCrossfade(transitionBars, nextSong.variation);
    };
    callbacks.onTransitionComplete = [this]() {
        // 過渡完成：crossfade 已結束，不需額外動作
    };
    callbacks.onSongChange = [this](int songIdx) {
        // 歌曲切換時的額外處理
    };
    transitionEngine_.setCallbacks(callbacks);

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

// === Density 控制 ===

void AudioEngine::setDensity(TechnoMachine::Role role, float density)
{
    patternEngine_.setDensity(role, density);
}

float AudioEngine::getDensity(TechnoMachine::Role role) const
{
    return patternEngine_.getDensity(role);
}

// === Playback Density（即時過濾）===

void AudioEngine::setPlaybackDensity(TechnoMachine::Role role, float density)
{
    if (role >= 0 && role < TechnoMachine::NUM_ROLES) {
        playbackDensity_[role] = std::clamp(density, 0.0f, 1.0f);
    }
}

float AudioEngine::getPlaybackDensity(TechnoMachine::Role role) const
{
    if (role >= 0 && role < TechnoMachine::NUM_ROLES) {
        return playbackDensity_[role];
    }
    return 1.0f;
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

void AudioEngine::generateRandomSet(int numSongs, int barsPerSong)
{
    transitionEngine_.getSongManager().generateRandomSet(numSongs, barsPerSong);
    transitionEngine_.initialize();

    // 套用第一首歌的設定
    const auto& song = transitionEngine_.getSongManager().getCurrentSong();
    patternEngine_.setStyle(song.styleIdx);
    regeneratePatterns(song.variation);
}

void AudioEngine::setSongDuration(int bars)
{
    transitionEngine_.getSongManager().setAllSongDuration(bars);
}

void AudioEngine::setTransitionDuration(int bars)
{
    transitionEngine_.getSongManager().setTransitionDuration(bars);
}

int AudioEngine::getTransitionDuration() const
{
    return transitionEngine_.getSongManager().getTransitionDuration();
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
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    // 使用 crossfade 決策（如果在過渡中）
    for (int v = 0; v < TechnoMachine::NUM_VOICES; v++) {
        auto decision = patternEngine_.getCrossfadeDecision(v, step);
        if (decision.shouldTrigger) {
            // 套用 playback density 過濾
            int role = v / 2;  // 每 2 個 voice 屬於一個 role
            float density = playbackDensity_[role];

            // density = 1.0 時全部播放，density = 0.0 時全部靜音
            if (density >= 1.0f || dist(densityRng_) < density) {
                drums_.triggerVoice(v, decision.velocity);
            }
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

        // 如果過渡中，推進 crossfade 進度
        if (transitionEngine_.isTransitioning()) {
            patternEngine_.notifyCrossfadeBarStart();
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
