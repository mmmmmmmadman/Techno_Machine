/**
 * StyleProfiles.hpp
 * Techno Machine - 10 種節奏風格定義
 *
 * 來源：UniversalRhythm StyleProfiles.hpp
 * 修改：所有風格 Techno 化（straight timing, 適合 4/4）
 */

#pragma once

#include "../Synthesis/MinimalDrumSynth.h"

namespace TechnoMachine {

/**
 * 風格設定檔結構
 */
struct StyleProfile {
    const char* name;
    float swing;  // 0.5 = straight, 0.67 = triplet

    // 16-position weights for each role (0.0 - 1.0)
    float timeline[16];
    float foundation[16];
    float groove[16];
    float lead[16];

    // Density ranges per role [min, max]
    float densityRange[NUM_ROLES][2];
};

// ============================================================
// STYLE 0: Techno (基礎)
// ============================================================
inline const StyleProfile STYLE_TECHNO = {
    "Techno",
    0.50f,

    // Timeline: Hi-hat dense but with gaps
    {1.0f, 0.8f, 1.0f, 0.0f, 1.0f, 0.8f, 1.0f, 0.0f,
     1.0f, 0.8f, 1.0f, 0.0f, 1.0f, 0.8f, 1.0f, 0.0f},

    // Foundation: Four-on-floor
    {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
     1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f},

    // Groove: Clap on 2 and 4
    {0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
     0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f},

    // Lead: Sparse industrial perc
    {0.0f, 0.8f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.9f,
     0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.7f},

    // Density ranges [Timeline, Foundation, Groove, Lead]
    {{0.50f, 0.90f}, {0.25f, 0.35f}, {0.12f, 0.25f}, {0.15f, 0.35f}}
};

// ============================================================
// STYLE 1: Electronic (House 變體)
// ============================================================
inline const StyleProfile STYLE_ELECTRONIC = {
    "Electronic",
    0.50f,

    // Timeline: Full 16th hi-hats
    {1.0f, 0.6f, 1.0f, 0.6f, 1.0f, 0.6f, 1.0f, 0.6f,
     1.0f, 0.6f, 1.0f, 0.6f, 1.0f, 0.6f, 1.0f, 0.6f},

    // Foundation: Four-on-floor
    {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
     1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f},

    // Groove: Snare/clap on 2 and 4
    {0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
     0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f},

    // Lead: Syncopated percussion
    {0.2f, 0.4f, 0.3f, 0.5f, 0.1f, 0.3f, 0.2f, 0.6f,
     0.2f, 0.4f, 0.3f, 0.5f, 0.1f, 0.3f, 0.2f, 0.5f},

    {{0.50f, 0.80f}, {0.25f, 0.30f}, {0.10f, 0.20f}, {0.20f, 0.40f}}
};

// ============================================================
// STYLE 2: Breakbeat (DnB/Breaks 風格)
// ============================================================
inline const StyleProfile STYLE_BREAKBEAT = {
    "Breakbeat",
    0.52f,

    // Timeline: Syncopated hat pattern
    {1.0f, 0.0f, 0.8f, 0.0f, 1.0f, 0.0f, 0.7f, 0.0f,
     1.0f, 0.0f, 0.8f, 0.0f, 1.0f, 0.0f, 0.7f, 0.0f},

    // Foundation: 2-step kick (syncopated)
    {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
     0.9f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.9f, 0.0f},

    // Groove: Snare on 2 and 4
    {0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
     0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f},

    // Lead: Breakbeat chops
    {0.2f, 0.4f, 0.3f, 0.5f, 0.1f, 0.4f, 0.3f, 0.6f,
     0.3f, 0.5f, 0.2f, 0.4f, 0.1f, 0.5f, 0.3f, 0.6f},

    {{0.40f, 0.65f}, {0.15f, 0.25f}, {0.10f, 0.20f}, {0.25f, 0.45f}}
};

// ============================================================
// STYLE 3: West African (Techno 化)
// ============================================================
// 保留 hemiola 3:2 張力，使用 straight timing
inline const StyleProfile STYLE_WEST_AFRICAN = {
    "West African",
    0.50f,  // Techno 化：straight timing

    // Timeline: Bell pattern adapted (hemiola accents)
    {1.0f, 0.0f, 0.0f, 0.9f, 0.0f, 0.0f, 0.9f, 1.0f,
     0.0f, 0.0f, 0.9f, 0.0f, 1.0f, 0.0f, 0.0f, 0.9f},

    // Foundation: Sparse, beat 1 dominant
    {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
     0.8f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},

    // Groove: Hemiola 3:2 structure
    {0.8f, 0.3f, 0.2f, 0.4f, 0.8f, 0.2f, 0.7f, 0.3f,
     0.8f, 0.2f, 0.3f, 0.3f, 0.7f, 0.3f, 0.2f, 0.2f},

    // Lead: Responds to hemiola tension
    {0.4f, 0.5f, 0.6f, 0.4f, 0.7f, 0.5f, 0.6f, 0.5f,
     0.7f, 0.5f, 0.4f, 0.6f, 0.5f, 0.6f, 0.5f, 0.4f},

    {{0.35f, 0.50f}, {0.08f, 0.15f}, {0.30f, 0.45f}, {0.20f, 0.35f}}
};

// ============================================================
// STYLE 4: Afro-Cuban (Techno 化)
// ============================================================
// 只保留 tumbao 節奏感，不使用 Clave timeline
inline const StyleProfile STYLE_AFRO_CUBAN = {
    "Afro-Cuban",
    0.50f,  // Techno 化：straight timing

    // Timeline: Simplified, not strict clave
    {1.0f, 0.0f, 0.0f, 0.8f, 0.0f, 0.0f, 0.9f, 0.0f,
     0.0f, 0.0f, 0.8f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f},

    // Foundation: Four-on-floor (Techno 化)
    {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
     1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f},

    // Groove: Tumbao-inspired syncopation
    {0.2f, 0.7f, 0.6f, 0.2f, 0.8f, 0.4f, 0.2f, 0.7f,
     0.6f, 0.4f, 0.2f, 0.7f, 0.2f, 0.6f, 0.4f, 0.3f},

    // Lead: Quinto-style improvisation
    {0.5f, 0.5f, 0.6f, 0.5f, 0.6f, 0.5f, 0.5f, 0.6f,
     0.5f, 0.6f, 0.5f, 0.5f, 0.6f, 0.5f, 0.5f, 0.5f},

    {{0.30f, 0.45f}, {0.25f, 0.30f}, {0.35f, 0.50f}, {0.15f, 0.35f}}
};

// ============================================================
// STYLE 5: Brazilian (Techno 化)
// ============================================================
// 採用 Tamborim 細碎節奏，Surdo 改為 four-on-floor
inline const StyleProfile STYLE_BRAZILIAN = {
    "Brazilian",
    0.50f,  // Techno 化：straight timing

    // Timeline: Agogo-inspired
    {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
     1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.7f, 0.0f},

    // Foundation: Four-on-floor (Techno 化，非 beat 2)
    {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
     1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f},

    // Groove: Tamborim-style busy pattern
    {0.3f, 0.7f, 0.4f, 0.7f, 0.2f, 0.6f, 0.5f, 0.7f,
     0.3f, 0.7f, 0.4f, 0.7f, 0.2f, 0.6f, 0.5f, 0.7f},

    // Lead: Repinique calls
    {0.3f, 0.2f, 0.3f, 0.2f, 0.5f, 0.2f, 0.3f, 0.2f,
     0.3f, 0.2f, 0.3f, 0.2f, 0.5f, 0.2f, 0.3f, 0.2f},

    {{0.25f, 0.40f}, {0.25f, 0.30f}, {0.45f, 0.60f}, {0.20f, 0.35f}}
};

// ============================================================
// STYLE 6: Jazz (Techno 化)
// ============================================================
// 保留 ride 節奏型態，使用 straight timing
inline const StyleProfile STYLE_JAZZ = {
    "Jazz",
    0.50f,  // Techno 化：straight timing（非 0.65）

    // Timeline: Ride pattern adapted
    {1.0f, 0.0f, 0.0f, 0.8f, 1.0f, 0.0f, 0.0f, 0.8f,
     1.0f, 0.0f, 0.0f, 0.8f, 1.0f, 0.0f, 0.0f, 0.0f},

    // Foundation: Sparse kick on 1 and 3
    {0.9f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
     0.8f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},

    // Groove: Snare comping (sparse, random feel)
    {0.1f, 0.2f, 0.2f, 0.5f, 0.1f, 0.2f, 0.2f, 0.5f,
     0.1f, 0.2f, 0.2f, 0.5f, 0.1f, 0.2f, 0.2f, 0.4f},

    // Lead: Ghost notes and accents
    {0.2f, 0.2f, 0.2f, 0.3f, 0.2f, 0.2f, 0.2f, 0.3f,
     0.2f, 0.2f, 0.2f, 0.3f, 0.2f, 0.3f, 0.3f, 0.4f},

    {{0.35f, 0.50f}, {0.10f, 0.18f}, {0.20f, 0.35f}, {0.15f, 0.30f}}
};

// ============================================================
// STYLE 7: Balkan (Techno 化)
// ============================================================
// 將 2+2+3 的不對稱張力映射到 16 步
inline const StyleProfile STYLE_BALKAN = {
    "Balkan",
    0.50f,

    // Timeline: Asymmetric accent groups (2+2+3 feel in 16 steps)
    // Groups: [1-4][5-8][9-16] with emphasis on group starts
    {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
     0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.8f, 0.0f},

    // Foundation: Group downbeats
    {1.0f, 0.0f, 0.0f, 0.0f, 0.9f, 0.0f, 0.0f, 0.0f,
     0.0f, 0.8f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},

    // Groove: Fill within groups
    {0.2f, 0.6f, 0.5f, 0.0f, 0.2f, 0.6f, 0.5f, 0.6f,
     0.0f, 0.2f, 0.6f, 0.5f, 0.6f, 0.0f, 0.5f, 0.0f},

    // Lead: Ornamental
    {0.3f, 0.4f, 0.5f, 0.0f, 0.3f, 0.4f, 0.5f, 0.0f,
     0.5f, 0.3f, 0.4f, 0.5f, 0.0f, 0.5f, 0.0f, 0.4f},

    {{0.18f, 0.30f}, {0.15f, 0.22f}, {0.35f, 0.50f}, {0.25f, 0.40f}}
};

// ============================================================
// STYLE 8: Indian (Techno 化)
// ============================================================
// 簡化為 Sam (強) vs Khali (空) 對比
inline const StyleProfile STYLE_INDIAN = {
    "Indian",
    0.50f,

    // Timeline: Teental theka adapted (Sam=1, Khali=9 light)
    {1.0f, 0.5f, 0.5f, 0.8f, 0.9f, 0.5f, 0.5f, 0.8f,
     0.1f, 0.5f, 0.5f, 0.7f, 0.9f, 0.5f, 0.5f, 0.8f},

    // Foundation: Sam and Tali emphasis
    {1.0f, 0.0f, 0.0f, 0.0f, 0.9f, 0.0f, 0.0f, 0.0f,
     0.0f, 0.0f, 0.0f, 0.0f, 0.8f, 0.0f, 0.0f, 0.0f},

    // Groove: Dayan elaboration
    {0.3f, 0.6f, 0.6f, 0.4f, 0.3f, 0.6f, 0.6f, 0.4f,
     0.2f, 0.5f, 0.5f, 0.4f, 0.3f, 0.6f, 0.6f, 0.4f},

    // Lead: Tihai-style build toward Sam
    {0.4f, 0.4f, 0.4f, 0.5f, 0.4f, 0.4f, 0.5f, 0.5f,
     0.3f, 0.4f, 0.5f, 0.5f, 0.6f, 0.6f, 0.7f, 0.8f},

    {{0.45f, 0.60f}, {0.15f, 0.22f}, {0.35f, 0.50f}, {0.25f, 0.40f}}
};

// ============================================================
// STYLE 9: Gamelan (Techno 化)
// ============================================================
// 保留 Kotekan 互鎖概念，提高密度
inline const StyleProfile STYLE_GAMELAN = {
    "Gamelan",
    0.50f,

    // Timeline: Colotomic punctuation (denser than original)
    {0.0f, 0.0f, 0.0f, 0.6f, 0.0f, 0.0f, 0.0f, 0.0f,
     0.0f, 0.0f, 0.0f, 0.6f, 0.0f, 0.0f, 0.0f, 1.0f},

    // Foundation: Gong punctuation + four-on-floor hybrid
    {1.0f, 0.0f, 0.0f, 0.0f, 0.8f, 0.0f, 0.0f, 0.0f,
     0.8f, 0.0f, 0.0f, 0.0f, 0.8f, 0.0f, 0.0f, 0.9f},

    // Groove: Kotekan polos (on-beat)
    {0.8f, 0.2f, 0.8f, 0.2f, 0.8f, 0.2f, 0.8f, 0.2f,
     0.8f, 0.2f, 0.8f, 0.2f, 0.8f, 0.2f, 0.8f, 0.3f},

    // Lead: Kotekan sangsih (off-beat) - interlocking
    {0.2f, 0.8f, 0.2f, 0.8f, 0.2f, 0.8f, 0.2f, 0.8f,
     0.2f, 0.8f, 0.2f, 0.8f, 0.2f, 0.8f, 0.2f, 0.7f},

    {{0.20f, 0.35f}, {0.22f, 0.30f}, {0.40f, 0.55f}, {0.40f, 0.55f}}
};

// ============================================================
// Style Array
// ============================================================

enum StyleType {
    STYLE_TYPE_TECHNO = 0,
    STYLE_TYPE_ELECTRONIC,
    STYLE_TYPE_BREAKBEAT,
    STYLE_TYPE_WEST_AFRICAN,
    STYLE_TYPE_AFRO_CUBAN,
    STYLE_TYPE_BRAZILIAN,
    STYLE_TYPE_JAZZ,
    STYLE_TYPE_BALKAN,
    STYLE_TYPE_INDIAN,
    STYLE_TYPE_GAMELAN,
    NUM_STYLES
};

inline const StyleProfile* const STYLES[] = {
    &STYLE_TECHNO,
    &STYLE_ELECTRONIC,
    &STYLE_BREAKBEAT,
    &STYLE_WEST_AFRICAN,
    &STYLE_AFRO_CUBAN,
    &STYLE_BRAZILIAN,
    &STYLE_JAZZ,
    &STYLE_BALKAN,
    &STYLE_INDIAN,
    &STYLE_GAMELAN
};

inline const char* getStyleName(int styleIdx) {
    if (styleIdx < 0 || styleIdx >= NUM_STYLES) return "Unknown";
    return STYLES[styleIdx]->name;
}

// ============================================================
// Style Dissimilarity Matrix
// ============================================================
// 值範圍 0.0-1.0：0.0 = 非常相似，1.0 = 非常不同
// 基於節奏結構、密度、重音位置分析

inline const float STYLE_DISSIMILARITY[NUM_STYLES][NUM_STYLES] = {
    //          Tech  Elec  Brkb  WAf   ACub  Braz  Jazz  Balk  Ind   Game
    /* Tech */ {0.0f, 0.2f, 0.6f, 0.8f, 0.5f, 0.6f, 0.7f, 0.7f, 0.6f, 0.9f},
    /* Elec */ {0.2f, 0.0f, 0.5f, 0.7f, 0.4f, 0.5f, 0.6f, 0.6f, 0.5f, 0.8f},
    /* Brkb */ {0.6f, 0.5f, 0.0f, 0.5f, 0.4f, 0.5f, 0.6f, 0.5f, 0.5f, 0.7f},
    /* WAf  */ {0.8f, 0.7f, 0.5f, 0.0f, 0.4f, 0.5f, 0.6f, 0.5f, 0.4f, 0.6f},
    /* ACub */ {0.5f, 0.4f, 0.4f, 0.4f, 0.0f, 0.3f, 0.5f, 0.5f, 0.4f, 0.7f},
    /* Braz */ {0.6f, 0.5f, 0.5f, 0.5f, 0.3f, 0.0f, 0.6f, 0.5f, 0.5f, 0.6f},
    /* Jazz */ {0.7f, 0.6f, 0.6f, 0.6f, 0.5f, 0.6f, 0.0f, 0.6f, 0.5f, 0.7f},
    /* Balk */ {0.7f, 0.6f, 0.5f, 0.5f, 0.5f, 0.5f, 0.6f, 0.0f, 0.4f, 0.6f},
    /* Ind  */ {0.6f, 0.5f, 0.5f, 0.4f, 0.4f, 0.5f, 0.5f, 0.4f, 0.0f, 0.5f},
    /* Game */ {0.9f, 0.8f, 0.7f, 0.6f, 0.7f, 0.6f, 0.7f, 0.6f, 0.5f, 0.0f}
};

/**
 * 取得兩個風格之間的差異度
 */
inline float getStyleDissimilarity(int styleA, int styleB) {
    if (styleA < 0 || styleA >= NUM_STYLES || styleB < 0 || styleB >= NUM_STYLES) {
        return 0.5f;
    }
    return STYLE_DISSIMILARITY[styleA][styleB];
}

/**
 * 找出與指定風格差異最大的風格列表
 * @param currentStyle 當前風格
 * @param minDissimilarity 最小差異度門檻
 * @param outStyles 輸出的風格索引陣列
 * @return 符合條件的風格數量
 */
inline int findDissimilarStyles(int currentStyle, float minDissimilarity, int* outStyles) {
    int count = 0;
    for (int i = 0; i < NUM_STYLES; i++) {
        if (i != currentStyle && getStyleDissimilarity(currentStyle, i) >= minDissimilarity) {
            outStyles[count++] = i;
        }
    }
    return count;
}

} // namespace TechnoMachine
