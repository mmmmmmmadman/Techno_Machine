/**
 * MarkovChain.hpp
 * Techno Machine - 馬可夫鏈節奏生成
 *
 * 使用轉移矩陣動態決定每步是否觸發
 * 溫度參數由 Density 控制
 */

#pragma once

#include <random>
#include <cmath>
#include <algorithm>

namespace TechnoMachine {

/**
 * 馬可夫鏈狀態
 */
enum class MarkovState {
    REST = 0,   // 休止
    HIT = 1     // 打擊
};

/**
 * 單一 Voice 的馬可夫鏈
 */
class MarkovChain {
public:
    MarkovChain() : state_(MarkovState::REST), rng_(std::random_device{}()) {
        // 預設轉移機率（平衡）
        setTransitionMatrix(0.3f, 0.5f);
    }

    /**
     * 設定轉移矩陣
     * @param restToHit 從休止到打擊的基礎機率
     * @param hitToHit 從打擊到打擊的基礎機率（連續打擊）
     */
    void setTransitionMatrix(float restToHit, float hitToHit) {
        baseRestToHit_ = std::clamp(restToHit, 0.0f, 1.0f);
        baseHitToHit_ = std::clamp(hitToHit, 0.0f, 1.0f);
    }

    /**
     * 根據風格權重設定轉移機率
     * @param weight 當前步驟的風格權重 (0-1)
     * @param density 密度參數 (0-1)，影響整體觸發傾向
     */
    void setStepWeight(float weight, float density) {
        currentWeight_ = std::clamp(weight, 0.0f, 1.0f);
        density_ = std::clamp(density, 0.0f, 1.0f);
    }

    /**
     * 設定溫度（由 Density 衍生）
     * 低溫 = 穩定、規律
     * 高溫 = 更多隨機性
     */
    void setTemperature(float temp) {
        temperature_ = std::clamp(temp, 0.1f, 2.0f);
    }

    /**
     * 執行一步轉移，返回是否應該觸發
     * @param fillActive Fill 是否啟用（提高溫度和觸發傾向）
     * @param fillIntensity Fill 強度 (0-1)
     * @return true = 觸發, false = 休止
     */
    bool step(bool fillActive = false, float fillIntensity = 0.0f) {
        // 計算有效轉移機率
        float restToHit = calculateTransitionProb(baseRestToHit_, true, fillActive, fillIntensity);
        float hitToHit = calculateTransitionProb(baseHitToHit_, false, fillActive, fillIntensity);

        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        float roll = dist(rng_);

        // 根據當前狀態決定轉移
        if (state_ == MarkovState::REST) {
            if (roll < restToHit) {
                state_ = MarkovState::HIT;
            }
        } else {
            if (roll < hitToHit) {
                state_ = MarkovState::HIT;  // 維持打擊
            } else {
                state_ = MarkovState::REST;
            }
        }

        return state_ == MarkovState::HIT;
    }

    /**
     * 取得當前狀態
     */
    MarkovState getState() const { return state_; }

    /**
     * 重設狀態
     */
    void reset() { state_ = MarkovState::REST; }

    /**
     * 強制設定狀態（用於同步）
     */
    void setState(MarkovState s) { state_ = s; }

private:
    MarkovState state_;
    std::mt19937 rng_;

    // 基礎轉移機率
    float baseRestToHit_ = 0.3f;
    float baseHitToHit_ = 0.5f;

    // 動態參數
    float currentWeight_ = 0.5f;
    float density_ = 0.5f;
    float temperature_ = 1.0f;

    /**
     * 計算實際轉移機率
     */
    float calculateTransitionProb(float baseProb, bool isRestToHit,
                                   bool fillActive, float fillIntensity) {
        float prob = baseProb;

        // 1. 套用風格權重
        prob *= currentWeight_;

        // 2. 套用 Density（影響整體觸發傾向）
        if (isRestToHit) {
            // REST → HIT: Density 越高越容易觸發
            prob *= (0.5f + density_);
        } else {
            // HIT → HIT: Density 影響連續打擊的可能性
            prob *= (0.7f + density_ * 0.6f);
        }

        // 3. 套用溫度（來自 Density）
        // 高溫 = 機率趨向 0.5（更隨機）
        // 低溫 = 機率保持原樣（更穩定）
        float tempFactor = 1.0f / temperature_;
        prob = 0.5f + (prob - 0.5f) * tempFactor;

        // 4. Fill 模式：提高觸發傾向
        if (fillActive) {
            float fillBoost = fillIntensity * 0.4f;
            prob += fillBoost;
        }

        return std::clamp(prob, 0.0f, 1.0f);
    }
};

/**
 * 8 Voice 馬可夫鏈管理器
 */
class MarkovEngine {
public:
    MarkovEngine() {
        // 為每個 Voice 設定不同的基礎轉移機率
        // Timeline (Hi-Hat): 高觸發率、高連續性
        chains_[0].setTransitionMatrix(0.6f, 0.7f);
        chains_[1].setTransitionMatrix(0.4f, 0.5f);

        // Foundation (Kick): 低觸發率、低連續性（四拍穩定）
        chains_[2].setTransitionMatrix(0.25f, 0.1f);
        chains_[3].setTransitionMatrix(0.15f, 0.1f);

        // Groove (Clap): 中等觸發、低連續性
        chains_[4].setTransitionMatrix(0.3f, 0.2f);
        chains_[5].setTransitionMatrix(0.2f, 0.15f);

        // Lead (Perc): 中等觸發、中等連續
        chains_[6].setTransitionMatrix(0.35f, 0.4f);
        chains_[7].setTransitionMatrix(0.25f, 0.3f);
    }

    /**
     * 更新所有鏈的步驟權重
     * @param step 當前步驟 (0-15)
     * @param weights 8 個 voice 的權重陣列
     * @param densities 4 個 role 的 density
     */
    void updateStepWeights(int step, const float* weights, const float* densities) {
        for (int v = 0; v < 8; v++) {
            int role = v / 2;
            chains_[v].setStepWeight(weights[v], densities[role]);
            // 溫度 = 0.5 + density（density 越高越隨機）
            chains_[v].setTemperature(0.5f + densities[role]);
        }
    }

    /**
     * 執行一步，返回 8 個 voice 的觸發結果
     */
    void step(bool* triggers, bool fillActive = false, float fillIntensity = 0.0f) {
        for (int v = 0; v < 8; v++) {
            triggers[v] = chains_[v].step(fillActive, fillIntensity);
        }
    }

    /**
     * 取得單一 voice 的馬可夫鏈
     */
    MarkovChain& getChain(int voice) {
        return chains_[voice % 8];
    }

    /**
     * 重設所有鏈
     */
    void reset() {
        for (int v = 0; v < 8; v++) {
            chains_[v].reset();
        }
    }

private:
    MarkovChain chains_[8];
};

} // namespace TechnoMachine
