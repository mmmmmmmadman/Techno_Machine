# Techno Machine - 安裝與使用說明

## 下載

前往 [GitHub Releases](https://github.com/mmmmmmmadman/Techno_Machine/releases) 下載最新版本：

- **macOS**: `Techno_Machine_macOS.zip`
- **Windows**: `Techno_Machine_Windows.zip`

---

## macOS 安裝

1. 下載 `Techno_Machine_macOS.zip`
2. 解壓縮（雙擊 .zip 檔案）
3. 將 `Techno Machine.app` 拖曳到「應用程式」資料夾
4. 首次執行時，若出現「無法打開」警告：
   - 右鍵點擊 App → 選擇「打開」
   - 或前往「系統設定」→「隱私與安全性」→ 點擊「仍要打開」

---

## Windows 安裝

1. 下載 `Techno_Machine_Windows.zip`
2. 解壓縮到任意位置
3. 雙擊 `Techno Machine.exe` 執行
4. 若出現 Windows Defender 警告，點擊「更多資訊」→「仍要執行」

---

## 使用介面

### 基本操作

| 按鈕 | 功能 |
|------|------|
| **Play** | 開始播放 |
| **Stop** | 停止並重置 |
| **Swing** | 切換 Swing 強度 (Off/1/2/3) |
| **Settings** | 開啟音訊與 CV 設定 |

### 混音控制

- **Timeline / Foundation / Groove / Lead**: 四個角色的音量
- **Density 滑桿**: 控制各角色的節奏密度
- **Global**: 整體密度偏移
- **Fill**: Fill 強度 (0-100%)

### DJ 模式

- **Load A / Load B**: 載入新節奏到 Deck A 或 B
- **Crossfader**: 在兩個 Deck 之間淡入淡出（DJ 風格曲線）
- **Build**: 長按啟動自動化 build-up（可選 4/8/16/32 bars）

---

## 功能說明 (v0.3.2)

### 馬可夫鏈序列器
- 為節奏 pattern 增加有機變化
- 溫度參數由 Density 控制
- Density 越高，隨機觸發越多
- 當 Density > 30% 時啟用

### Build-up 自動化
- 長按 Build 按鈕開始自動化
- Fill Intensity 逐漸上升至最大值
- Global Density 緩慢增加
- Fill 間隔逐漸縮短
- 放開按鈕瞬間恢復（Drop 效果）

### 持續過門模式
- 將 Fill Intensity 設為 100% 即可持續過門
- Fill pattern 會一直播放直到降低強度

---

## CV 輸出設定

適用於 DC-coupled 音訊介面（如 Expert Sleepers ES-8）。

1. 點擊 **Settings** 按鈕
2. 在 **Audio Device** 頁籤選擇多通道音訊介面
3. 切換到 **CV Routing** 頁籤
4. 為每個 CV 信號選擇輸出通道

### CV 信號類型

| 類型 | 說明 |
|------|------|
| **Trig** | 1ms 觸發脈衝 (0-1V) |
| **Pitch** | 音高 CV，1V/Oct 標準 |
| **Vel** | 力度 CV (0-1V) |

### 通道配置建議

- 通道 0-1: 立體聲音訊輸出
- 通道 2+: CV 輸出

---

## 系統需求

### macOS
- macOS 10.15 (Catalina) 或更新版本
- Apple Silicon 或 Intel 處理器

### Windows
- Windows 10 或更新版本
- 64-bit 處理器

### 音訊介面
- 任何 Core Audio (macOS) 或 ASIO (Windows) 介面
- CV 輸出需要 DC-coupled 介面

---

## 疑難排解

### 沒有聲音
1. 開啟 Settings → Audio Device
2. 確認已選擇正確的輸出裝置
3. 確認 Sample Rate 與介面相符

### CV 沒有輸出
1. 確認音訊介面支援多通道輸出
2. 在 CV Routing 頁籤設定正確的通道
3. 通道 0-1 保留給音訊，CV 從通道 2 開始

### macOS 無法開啟 App
右鍵 → 打開，或在「系統設定」→「隱私與安全性」允許

---

## 聯絡

- GitHub: https://github.com/mmmmmmmadman/Techno_Machine
- Issues: https://github.com/mmmmmmmadman/Techno_Machine/issues
