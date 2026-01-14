/**
 * MinimalDrumSynth.h
 * Techno Machine - 4 聲道打擊樂合成引擎
 *
 * 架構：
 * - 4 Role = 4 合成器聲道
 * - Pattern 系統保留 8 個（Primary + Secondary Interlock）
 * - 觸發時 Primary 優先，合併輸出到單一聲道
 * - Velocity 可調變 Freq/Decay（用戶自訂演算法）
 *
 * 音色配置：
 * - TIMELINE: HiHat
 * - FOUNDATION: Kick
 * - GROOVE: Clap
 * - LEAD: Perc
 */

#pragma once

#include <cmath>
#include <random>

// MSVC compatibility
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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
    LEAD = 3,        // Perc
    NUM_ROLES = 4
};

// Pattern 系統用（保留 8 個 pattern 的 Interlock 邏輯）
static constexpr int NUM_PATTERN_VOICES = 8;

// 合成器聲道數（每 Role 一個）
static constexpr int NUM_VOICES = 4;

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
    float getFreq() const { return freq_; }

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
 * Techno 風格 4 聲道預設
 * 每個 Role 一個基礎音色
 */
inline const VoicePreset TECHNO_PRESETS[NUM_VOICES] = {
    { SynthMode::NOISE, 10000.0f, 20.0f },   // TIMELINE: HiHat
    { SynthMode::SINE, 42.0f, 250.0f },      // FOUNDATION: 909 Kick
    { SynthMode::NOISE, 1800.0f, 55.0f },    // GROOVE: Clap
    { SynthMode::NOISE, 5000.0f, 80.0f }     // LEAD: Perc
};

/**
 * 10 種風格的音色預設
 * 每種風格 4 個音色（每 Role 一個）
 */
inline const VoicePreset STYLE_PRESETS[10][NUM_VOICES] = {
    // 0: TECHNO - 經典 909 電子音色
    {
        { SynthMode::NOISE, 10000.0f, 20.0f },   // TIMELINE: HiHat
        { SynthMode::SINE, 42.0f, 250.0f },      // FOUNDATION: 909 Kick
        { SynthMode::NOISE, 1800.0f, 55.0f },    // GROOVE: Clap
        { SynthMode::NOISE, 5000.0f, 80.0f }     // LEAD: Open HH
    },
    // 1: ELECTRONIC - 更亮更銳利
    {
        { SynthMode::NOISE, 14000.0f, 15.0f },   // TIMELINE: HiHat
        { SynthMode::SINE, 38.0f, 300.0f },      // FOUNDATION: Kick
        { SynthMode::NOISE, 2200.0f, 40.0f },    // GROOVE: Clap
        { SynthMode::NOISE, 6000.0f, 100.0f }    // LEAD: Open HH
    },
    // 2: BREAKBEAT - 更有機、斷裂感
    {
        { SynthMode::NOISE, 8000.0f, 35.0f },    // TIMELINE: HiHat
        { SynthMode::SINE, 55.0f, 180.0f },      // FOUNDATION: Kick
        { SynthMode::NOISE, 1200.0f, 80.0f },    // GROOVE: Snare
        { SynthMode::NOISE, 4000.0f, 120.0f }    // LEAD: Open HH
    },
    // 3: WEST_AFRICAN - 手鼓、Djembe 風格
    {
        { SynthMode::NOISE, 5000.0f, 45.0f },    // TIMELINE: Shaker
        { SynthMode::SINE, 80.0f, 150.0f },      // FOUNDATION: Djembe Bass
        { SynthMode::NOISE, 800.0f, 70.0f },     // GROOVE: Djembe Slap
        { SynthMode::NOISE, 3000.0f, 60.0f }     // LEAD: Shekere
    },
    // 4: AFRO_CUBAN - Conga、Bongo
    {
        { SynthMode::NOISE, 4500.0f, 50.0f },    // TIMELINE: Guiro
        { SynthMode::SINE, 90.0f, 180.0f },      // FOUNDATION: Conga Low
        { SynthMode::NOISE, 1000.0f, 60.0f },    // GROOVE: Conga Slap
        { SynthMode::NOISE, 2500.0f, 40.0f }     // LEAD: Timbale
    },
    // 5: BRAZILIAN - Samba 風格
    {
        { SynthMode::NOISE, 7000.0f, 25.0f },    // TIMELINE: Tamborim
        { SynthMode::SINE, 65.0f, 200.0f },      // FOUNDATION: Surdo
        { SynthMode::NOISE, 1500.0f, 45.0f },    // GROOVE: Caixa
        { SynthMode::NOISE, 4500.0f, 55.0f }     // LEAD: Cuica
    },
    // 6: JAZZ - Brush、Ride 風格
    {
        { SynthMode::NOISE, 6000.0f, 90.0f },    // TIMELINE: Ride
        { SynthMode::SINE, 50.0f, 350.0f },      // FOUNDATION: Kick
        { SynthMode::NOISE, 1400.0f, 100.0f },   // GROOVE: Brush
        { SynthMode::NOISE, 5500.0f, 150.0f }    // LEAD: Crash
    },
    // 7: BALKAN - 不對稱、金屬感
    {
        { SynthMode::NOISE, 9000.0f, 30.0f },    // TIMELINE: Zurna-like
        { SynthMode::SINE, 75.0f, 130.0f },      // FOUNDATION: Tapan Bass
        { SynthMode::NOISE, 2000.0f, 40.0f },    // GROOVE: Tapan Rim
        { SynthMode::NOISE, 3500.0f, 45.0f }     // LEAD: Darbuka
    },
    // 8: INDIAN - Tabla 風格
    {
        { SynthMode::NOISE, 5500.0f, 40.0f },    // TIMELINE: Jhanjh
        { SynthMode::SINE, 60.0f, 280.0f },      // FOUNDATION: Bayan
        { SynthMode::SINE, 200.0f, 60.0f },      // GROOVE: Dayan
        { SynthMode::NOISE, 2800.0f, 50.0f }     // LEAD: Dholak
    },
    // 9: GAMELAN - 金屬、鐘聲
    {
        { SynthMode::SINE, 1200.0f, 200.0f },    // TIMELINE: Kenong
        { SynthMode::SINE, 100.0f, 400.0f },     // FOUNDATION: Gong
        { SynthMode::SINE, 800.0f, 120.0f },     // GROOVE: Saron
        { SynthMode::SINE, 2000.0f, 80.0f }      // LEAD: Bonang
    }
};

/**
 * 取得指定風格的音色預設
 */
inline const VoicePreset* getStylePreset(int styleIdx) {
    if (styleIdx < 0 || styleIdx >= 10) styleIdx = 0;
    return STYLE_PRESETS[styleIdx];
}

/**
 * 4 聲道打擊樂合成器（每 Role 一個聲道）
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
     * voiceIdx: 0-3（對應 4 個 Role）
     */
    void setVoiceParams(int voiceIdx, SynthMode mode, float freq, float decay) {
        if (voiceIdx < 0 || voiceIdx >= NUM_VOICES) return;
        voices_[voiceIdx].setMode(mode);
        voices_[voiceIdx].setFreq(freq);
        voices_[voiceIdx].setDecay(decay);
    }

    /**
     * 觸發某個 Role
     * role: 0-3
     * velocity: 觸發力度
     */
    void triggerRole(Role role, float velocity) {
        if (role < 0 || role >= NUM_ROLES) return;
        if (velocity > 0.0f) voices_[role].trigger(velocity);
    }

    /**
     * 觸發單一 Voice（與 triggerRole 相同，保留相容性）
     * voiceIdx: 0-3
     */
    void triggerVoice(int voiceIdx, float velocity = 1.0f) {
        if (voiceIdx < 0 || voiceIdx >= NUM_VOICES) return;
        voices_[voiceIdx].trigger(velocity);
    }

    /**
     * 取得 Voice 的頻率（用於 CV 輸出）
     */
    float getVoiceFrequency(int voiceIdx) const {
        if (voiceIdx < 0 || voiceIdx >= NUM_VOICES) return 440.0f;
        return voices_[voiceIdx].getFreq();
    }

    /**
     * 設定 Role 音量
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
            float signal = voices_[r].process();
            float pan = rolePan_[r];

            // Panning
            mixL += signal * levels_[r] * (0.5f - pan * 0.5f) * 1.414f;
            mixR += signal * levels_[r] * (0.5f + pan * 0.5f) * 1.414f;
        }

        // 軟限幅
        return {
            std::tanh(mixL * 0.7f),
            std::tanh(mixR * 0.7f)
        };
    }

    /**
     * 處理並輸出 4 個獨立聲道
     */
    void processSeparate(float* outputs) {
        for (int r = 0; r < NUM_ROLES; r++) {
            outputs[r] = voices_[r].process() * levels_[r];
        }
    }

private:
    MinimalVoice voices_[NUM_VOICES];
    float sampleRate_ = 48000.0f;

    // 音量（per Role）
    float levels_[NUM_ROLES] = { 0.5f, 1.0f, 0.7f, 0.5f };

    // Panning - 每個 Role 的立體聲位置
    // 範圍 -1.0 (左) 到 +1.0 (右)
    const float rolePan_[NUM_ROLES] = { 0.40f, 0.0f, -0.30f, -0.60f };
};

} // namespace TechnoMachine
