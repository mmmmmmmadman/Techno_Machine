# Techno Machine 安裝與使用說明

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

## 使用方式

### 基本操作

| 按鈕 | 功能 |
|------|------|
| **Play** | 開始播放 |
| **Stop** | 停止並重置 |
| **Swing** | 切換 Swing 強度 (Off/1/2/3) |
| **Settings** | 開啟音訊與 CV 設定 |

### 混音控制

- **Timeline / Foundation / Groove / Lead**: 四個音色的音量
- **Density 區域**: 控制各音色的節奏密度
- **Global**: 整體密度偏移

### DJ 模式

- **Load A / Load B**: 載入新節奏到 Deck A 或 B
- **Crossfader**: 在兩個 Deck 之間淡入淡出

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
