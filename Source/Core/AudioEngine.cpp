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
    sampleEngine_.prepare(sampleRate);

    // 套用 Techno 預設音色（8 聲道）
    drums_.applyTechnoPreset();

    // 設定預設音量
    using Role = TechnoMachine::Role;
    drums_.setLevel(Role::TIMELINE, 0.5f);    // Hi-Hat
    drums_.setLevel(Role::FOUNDATION, 1.0f);  // Kick
    drums_.setLevel(Role::GROOVE, 0.7f);      // Clap
    drums_.setLevel(Role::LEAD, 0.5f);        // Lead perc

    // 初始化 TransitionEngine（保留用於歌曲管理）
    transitionEngine_.initialize();

    // 初始化雙 Deck 系統
    patternEngine_.initializeDecks(16, 0.3f, 0.6f);

    // 載入第一首歌到 Deck A
    const auto& song = transitionEngine_.getSongManager().getCurrentSong();
    patternEngine_.loadToDeck(0, song.compositeStyle.roleStyles, song.variation);

    // 載入第二首歌到 Deck B（預備）
    const auto& nextSong = transitionEngine_.getSongManager().getNextSong();
    patternEngine_.loadToDeck(1, nextSong.compositeStyle.roleStyles, nextSong.variation);

    // Crossfader 設為 Deck A
    patternEngine_.setCrossfader(0.0f);

    // 套用初始音色
    applySynthModifiers();

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
    // 使用 crossfader 混合後的音色預設（直接混合風格參數）
    const auto mixed = patternEngine_.getMixedPresets();

    // 直接套用混合後的參數
    for (int v = 0; v < TechnoMachine::NUM_VOICES; v++) {
        drums_.setVoiceParams(v, mixed.mode[v], mixed.freq[v], mixed.decay[v]);
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

void AudioEngine::setFillIntensity(float intensity)
{
    patternEngine_.setFillIntensity(intensity);
}

float AudioEngine::getFillIntensity() const
{
    return patternEngine_.getFillIntensity();
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

    // 套用第一首歌的複合風格設定
    const auto& song = transitionEngine_.getSongManager().getCurrentSong();
    patternEngine_.setCompositeStyle(song.compositeStyle.roleStyles);
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

    // 套用新歌的複合風格設定
    const auto& song = transitionEngine_.getSongManager().getCurrentSong();
    patternEngine_.setCompositeStyle(song.compositeStyle.roleStyles);
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

    // 使用 Deck A/B 混音決策（根據 crossfader 位置）
    for (int v = 0; v < TechnoMachine::NUM_VOICES; v++) {
        auto decision = patternEngine_.getMixDecision(v, step);
        if (decision.shouldTrigger) {
            // 套用 playback density 過濾
            int role = v / 2;  // 每 2 個 voice 屬於一個 role
            float density = playbackDensity_[role];

            // density = 1.0 時全部播放，density = 0.0 時全部靜音
            if (density >= 1.0f || dist(densityRng_) < density) {
                // 如果此 voice 有 sample 則觸發 sample
                if (sampleEngine_.hasSample(v)) {
                    sampleEngine_.triggerVoice(v, decision.velocity);
                }
                // 同時觸發合成器（可疊加）
                drums_.triggerVoice(v, decision.velocity);

                // 設定 CV 觸發旗標
                voiceTriggered_[v] = true;
                lastVelocity_[v] = decision.velocity;
            }
        }
    }

    // 推進 Fill 計數器
    patternEngine_.advanceStep();
}

// === 手動 Crossfader 控制 ===

void AudioEngine::setCrossfader(float position)
{
    patternEngine_.setCrossfader(position);

    // 即時更新音色參數
    applySynthModifiers();
}

float AudioEngine::getCrossfader() const
{
    return patternEngine_.getCrossfader();
}

void AudioEngine::loadNextSong()
{
    // 推進 SongManager 到下一首
    transitionEngine_.getSongManager().advanceToNextSong();

    // 取得新歌資訊
    const auto& nextSong = transitionEngine_.getSongManager().getNextSong();

    // 載入到非作用中的 Deck
    patternEngine_.loadNextSong(nextSong.compositeStyle.roleStyles, nextSong.variation);
}

const char* AudioEngine::getDeckAStyleName() const
{
    return patternEngine_.getDeckStyleName(0);
}

const char* AudioEngine::getDeckBStyleName() const
{
    return patternEngine_.getDeckStyleName(1);
}

const char* AudioEngine::getDeckRoleStyleName(int deck, TechnoMachine::Role role) const
{
    return patternEngine_.getDeckRoleStyleName(deck, role);
}

void AudioEngine::loadToDeck(int deck)
{
    // 生成隨機複合風格
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<int> styleDist(0, TechnoMachine::NUM_STYLES - 1);
    std::uniform_real_distribution<float> varDist(0.2f, 0.7f);

    int roleStyles[TechnoMachine::NUM_ROLES];
    for (int i = 0; i < TechnoMachine::NUM_ROLES; i++) {
        roleStyles[i] = styleDist(rng);
    }

    float variation = varDist(rng);
    patternEngine_.loadToDeck(deck, roleStyles, variation);

    // 如果載入的是當前作用中的 Deck，更新音色
    if ((deck == 0 && patternEngine_.getCrossfader() < 0.5f) ||
        (deck == 1 && patternEngine_.getCrossfader() >= 0.5f)) {
        applySynthModifiers();
    }
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

    // Process all drum voices
    auto synthOutput = drums_.process();

    // Process samples
    auto sampleOutput = sampleEngine_.process();

    // Mix synth + samples
    return {
        synthOutput.left + sampleOutput.left,
        synthOutput.right + sampleOutput.right
    };
}

// === CV 輸出支援 ===

bool AudioEngine::wasVoiceTriggered(int voiceIdx) const
{
    if (voiceIdx < 0 || voiceIdx >= TechnoMachine::NUM_VOICES) return false;
    return voiceTriggered_[voiceIdx];
}

float AudioEngine::getLastVelocity(int voiceIdx) const
{
    if (voiceIdx < 0 || voiceIdx >= TechnoMachine::NUM_VOICES) return 0.0f;
    return lastVelocity_[voiceIdx];
}

void AudioEngine::clearTriggerFlags()
{
    for (int i = 0; i < TechnoMachine::NUM_VOICES; i++) {
        voiceTriggered_[i] = false;
    }
}

// === Sample 控制 ===

bool AudioEngine::loadSample(int voiceIdx, const juce::File& file)
{
    return sampleEngine_.loadSample(voiceIdx, file);
}

void AudioEngine::clearSample(int voiceIdx)
{
    sampleEngine_.clearSample(voiceIdx);
}

bool AudioEngine::hasSample(int voiceIdx) const
{
    return sampleEngine_.hasSample(voiceIdx);
}

juce::String AudioEngine::getSampleName(int voiceIdx) const
{
    return sampleEngine_.getSampleName(voiceIdx);
}

juce::String AudioEngine::getSamplePath(int voiceIdx) const
{
    return sampleEngine_.getSamplePath(voiceIdx);
}
