/**
 * MinimalDrumSynth.h
 * Techno Machine - 8 聲道打擊樂合成引擎
 *
 * 完全套用 UniversalRhythm 設計理念：
 * - 4 Role × 2 Voice = 8 聲道
 * - 只有 2 個參數：Freq 和 Decay
 * - SINE 模式：Freq 控制振盪頻率
 * - NOISE 模式：Freq 控制 BPF 中心頻率（已修正增益）
 * - Attack 固定超快 (<1ms) 確保瞬態
 * - Velocity 直接映射到 VCA 增益
 *
 * Techno 雙音色配置（來自 EXTENDED_PRESETS[9]）：
 * - TIMELINE: HiHat (10kHz) + HiHat Accent (12kHz)
 * - FOUNDATION: 909 Kick (42Hz) + Kick Layer (55Hz)
 * - GROOVE: Clap (1800Hz) + Rim (3000Hz)
 * - LEAD: Open HH (5kHz) + Tom (600Hz)
 */

#pragma once

#include <cmath>
#include <random>

namespace TechnoMachine {

// 合成器模式
enum class SynthMode {
    SINE,   // 音調類：Kick, Tom
    NOISE   // 噪音類：Hi-Hat, Clap, Rim
};

// 四角色定義
enum Role {
    TIMELINE = 0,    // Hi-Hat
    FOUNDATION = 1,  // Kick
    GROOVE = 2,      // Clap
    LEAD = 3,        // Rim
    NUM_ROLES = 4
};

static constexpr int NUM_VOICES = 8;  // 4 Role × 2 Voice

/**
 * 極簡單聲道合成器
 * 修正：BPF 使用正確的 0dB peak gain 公式
 */
class MinimalVoice {
public:
    void setSampleRate(float sr) {
        sampleRate_ = sr;
        cachedFreq_ = -1.0f;  // 強制更新 BPF 係數
    }

    void setMode(SynthMode m) { mode_ = m; }
    void setFreq(float f) { freq_ = std::max(20.0f, std::min(f, 20000.0f)); }
    void setDecay(float d) { decay_ = std::max(1.0f, std::min(d, 5000.0f)); }

    /**
     * 觸發音符
     * velocity 影響：音量 + 衰減長度
     */
    void trigger(float vel = 1.0f) {
        velocity_ = std::max(0.0f, std::min(vel, 1.0f));
        envValue_ = velocity_;  // 直接跳到峰值（超快攻擊）
        // 從 0.25 相位開始 = sin(π/2) = 1.0，產生瞬間 click
        phase_ = 0.25f;
        // 重置濾波器狀態
        bpfZ1_ = bpfZ2_ = 0.0f;
        // velocity 影響 decay 長度
        float velScale = 0.1f + 0.9f * std::pow(velocity_, 1.5f);
        actualDecay_ = decay_ * velScale;
    }

    float process() {
        if (envValue_ < 0.0001f) return 0.0f;

        float output = 0.0f;

        if (mode_ == SynthMode::SINE) {
            // === Sine 模式 ===
            output = std::sin(2.0f * static_cast<float>(M_PI) * phase_);
            phase_ += freq_ / sampleRate_;
            if (phase_ >= 1.0f) phase_ -= 1.0f;
        } else {
            // === Noise + BPF 模式 ===
            float noise = noiseDist_(rng_);
            output = processBPF(noise);
        }

        // VCA 包絡（指數衰減）
        float decaySamples = (actualDecay_ / 1000.0f) * sampleRate_;
        float decayCoef = std::exp(-1.0f / decaySamples);
        envValue_ *= decayCoef;

        return output * envValue_;
    }

private:
    void updateBPFCoefficients() {
        if (std::abs(freq_ - cachedFreq_) < 0.01f && std::abs(sampleRate_ - cachedSampleRate_) < 0.01f) return;

        float omega = 2.0f * static_cast<float>(M_PI) * freq_ / sampleRate_;
        float sinOmega = std::sin(omega);
        float cosOmega = std::cos(omega);
        float alpha = sinOmega / (2.0f * BPF_Q);

        float a0 = 1.0f + alpha;

        // 修正：使用 constant 0dB peak gain BPF 公式
        // b0 = Q * alpha = sin(omega) / 2
        // 這確保中心頻率增益為 0dB
        bpf_b0_ = (sinOmega / 2.0f) / a0;
        bpf_b1_ = 0.0f;
        bpf_b2_ = -(sinOmega / 2.0f) / a0;
        bpf_a1_ = (-2.0f * cosOmega) / a0;
        bpf_a2_ = (1.0f - alpha) / a0;

        cachedFreq_ = freq_;
        cachedSampleRate_ = sampleRate_;
    }

    float processBPF(float input) {
        updateBPFCoefficients();

        float w = input - bpf_a1_ * bpfZ1_ - bpf_a2_ * bpfZ2_;
        float output = bpf_b0_ * w + bpf_b1_ * bpfZ1_ + bpf_b2_ * bpfZ2_;

        bpfZ2_ = bpfZ1_;
        bpfZ1_ = w;

        return output;
    }

    // 狀態
    float sampleRate_ = 48000.0f;
    float phase_ = 0.0f;
    float envValue_ = 0.0f;
    float velocity_ = 1.0f;
    float actualDecay_ = 200.0f;

    // 參數
    SynthMode mode_ = SynthMode::SINE;
    float freq_ = 100.0f;
    float decay_ = 200.0f;

    // BPF 狀態
    float bpfZ1_ = 0.0f, bpfZ2_ = 0.0f;
    float cachedFreq_ = -1.0f, cachedSampleRate_ = -1.0f;
    float bpf_b0_ = 0.0f, bpf_b1_ = 0.0f, bpf_b2_ = 0.0f;
    float bpf_a1_ = 0.0f, bpf_a2_ = 0.0f;
    static constexpr float BPF_Q = 2.0f;

    // 噪音生成器
    std::mt19937 rng_{std::random_device{}()};
    std::uniform_real_distribution<float> noiseDist_{-1.0f, 1.0f};
};

/**
 * 音色預設結構
 */
struct VoicePreset {
    SynthMode mode;
    float freq;
    float decay;
};

/**
 * Techno 風格 8 聲道預設
 * 來自 UniversalRhythm EXTENDED_PRESETS[9]
 */
inline const VoicePreset TECHNO_PRESETS[NUM_VOICES] = {
    // TIMELINE: Voice 0, 1
    { SynthMode::NOISE, 10000.0f, 20.0f },   // HiHat
    { SynthMode::NOISE, 12000.0f, 12.0f },   // HiHat Accent
    // FOUNDATION: Voice 2, 3
    { SynthMode::SINE, 42.0f, 250.0f },      // 909 Kick
    { SynthMode::SINE, 55.0f, 180.0f },      // Kick Layer
    // GROOVE: Voice 4, 5
    { SynthMode::NOISE, 1800.0f, 55.0f },    // Clap
    { SynthMode::NOISE, 3000.0f, 35.0f },    // Rim
    // LEAD: Voice 6, 7
    { SynthMode::NOISE, 5000.0f, 80.0f },    // Open HH
    { SynthMode::SINE, 600.0f, 60.0f }       // Tom
};

/**
 * 8 聲道打擊樂合成器（4 Role × 2 Voice）
 */
class MinimalDrumSynth {
public:
    void setSampleRate(float sr) {
        sampleRate_ = sr;
        for (int i = 0; i < NUM_VOICES; i++) {
            voices_[i].setSampleRate(sr);
        }
    }

    /**
     * 套用 Techno 預設音色
     */
    void applyTechnoPreset() {
        for (int i = 0; i < NUM_VOICES; i++) {
            voices_[i].setMode(TECHNO_PRESETS[i].mode);
            voices_[i].setFreq(TECHNO_PRESETS[i].freq);
            voices_[i].setDecay(TECHNO_PRESETS[i].decay);
        }
    }

    /**
     * 設定某個 Voice 的音色
     */
    void setVoiceParams(int voiceIdx, SynthMode mode, float freq, float decay) {
        if (voiceIdx < 0 || voiceIdx >= NUM_VOICES) return;
        voices_[voiceIdx].setMode(mode);
        voices_[voiceIdx].setFreq(freq);
        voices_[voiceIdx].setDecay(decay);
    }

    /**
     * 觸發某個 Role（同時觸發兩個 Voice）
     * velocityV1: 主音色的 velocity
     * velocityV2: 副音色的 velocity（通常較低或不觸發）
     */
    void triggerRole(Role role, float velocityV1, float velocityV2 = 0.0f) {
        if (role < 0 || role >= NUM_ROLES) return;
        int voiceBase = role * 2;
        if (velocityV1 > 0.0f) voices_[voiceBase].trigger(velocityV1);
        if (velocityV2 > 0.0f) voices_[voiceBase + 1].trigger(velocityV2);
    }

    /**
     * 觸發單一 Voice
     */
    void triggerVoice(int voiceIdx, float velocity = 1.0f) {
        if (voiceIdx < 0 || voiceIdx >= NUM_VOICES) return;
        voices_[voiceIdx].trigger(velocity);
    }

    /**
     * 設定 Role 音量（影響該 Role 的兩個 Voice）
     */
    void setLevel(Role role, float level) {
        if (role < 0 || role >= NUM_ROLES) return;
        levels_[role] = std::max(0.0f, std::min(level, 2.0f));
    }

    /**
     * 處理並輸出立體聲混音
     */
    struct StereoOutput {
        float left;
        float right;
    };

    StereoOutput process() {
        float mixL = 0.0f, mixR = 0.0f;

        for (int r = 0; r < NUM_ROLES; r++) {
            int voiceBase = r * 2;

            // 處理兩個 Voice
            float signal1 = voices_[voiceBase].process();
            float signal2 = voices_[voiceBase + 1].process();

            // Pan 位置（套用 UniversalRhythm spread 全滿）
            float pan1 = rolePanV1_[r];
            float pan2 = rolePanV2_[r];

            // Voice 1 panning
            mixL += signal1 * levels_[r] * (0.5f - pan1 * 0.5f) * 1.414f;
            mixR += signal1 * levels_[r] * (0.5f + pan1 * 0.5f) * 1.414f;

            // Voice 2 panning
            mixL += signal2 * levels_[r] * (0.5f - pan2 * 0.5f) * 1.414f;
            mixR += signal2 * levels_[r] * (0.5f + pan2 * 0.5f) * 1.414f;
        }

        // 軟限幅
        return {
            std::tanh(mixL * 0.7f),
            std::tanh(mixR * 0.7f)
        };
    }

    /**
     * 處理並輸出 8 個獨立聲道
     */
    void processSeparate(float* outputs) {
        for (int r = 0; r < NUM_ROLES; r++) {
            int voiceBase = r * 2;
            outputs[voiceBase] = voices_[voiceBase].process() * levels_[r];
            outputs[voiceBase + 1] = voices_[voiceBase + 1].process() * levels_[r];
        }
    }

private:
    MinimalVoice voices_[NUM_VOICES];
    float sampleRate_ = 48000.0f;

    // 音量（per Role）
    float levels_[NUM_ROLES] = { 0.5f, 1.0f, 0.7f, 0.5f };

    // Panning - 套用 UniversalRhythm spread 全滿
    // 範圍 -1.0 (左) 到 +1.0 (右)
    // Timeline: 右側, Foundation: 中央, Groove: 分左右, Lead: 左側
    const float rolePanV1_[NUM_ROLES] = { 0.40f,  0.0f, -0.60f, -0.80f };
    const float rolePanV2_[NUM_ROLES] = { 0.50f,  0.0f,  0.60f, -1.00f };
};

} // namespace TechnoMachine
