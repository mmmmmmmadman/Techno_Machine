# Techno Machine - インストール・取扱説明書

## ダウンロード

[GitHub Releases](https://github.com/mmmmmmmadman/Techno_Machine/releases) から最新版をダウンロード：

- **macOS**: `Techno_Machine_macOS.zip`
- **Windows**: `Techno_Machine_Windows.zip`

---

## macOS インストール

1. `Techno_Machine_macOS.zip` をダウンロード
2. ダブルクリックして解凍
3. `Techno Machine.app` を「アプリケーション」フォルダにドラッグ
4. 初回起動時に「開けません」と表示された場合：
   - アプリを右クリック →「開く」を選択
   - または「システム設定」→「プライバシーとセキュリティ」→「このまま開く」をクリック

---

## Windows インストール

1. `Techno_Machine_Windows.zip` をダウンロード
2. 任意の場所に解凍
3. `Techno Machine.exe` をダブルクリックして実行
4. Windows Defender の警告が表示された場合、「詳細情報」→「実行」をクリック

---

## ユーザーインターフェース

### 基本操作

| ボタン | 機能 |
|--------|------|
| **Play** | 再生開始 |
| **Stop** | 停止してリセット |
| **Swing** | スウィング強度を切り替え (Off/1/2/3) |
| **Settings** | オーディオ・CV設定を開く |

### ミキサーセクション

- **Timeline / Foundation / Groove / Lead**: 各ドラムロールの音量
- **Density スライダー**: 各ロールのリズム密度を調整
- **Global**: 全体の密度オフセット
- **Fill**: フィル強度 (0-100%)

### DJモード

- **Load A / Load B**: Deck A または B に新しいリズムパターンを読み込む
- **Crossfader**: 2つのデッキ間をDJスタイルのカーブでブレンド
- **Build**: 長押しでビルドアップ自動化（4/8/16/32小節選択可能）

---

## 機能説明 (v0.3.2)

### マルコフ連鎖シーケンサー
- リズムパターンに有機的な変化を追加
- 温度パラメータはDensityで制御
- Densityが高いほどランダムトリガーが増加
- Density > 30% で有効化

### ビルドアップ自動化
- Buildボタンを長押しで自動化開始
- Fill Intensityが最大値まで上昇
- Global Densityが徐々に増加
- Fill間隔が段階的に短縮
- ボタンを離すと瞬時に元に戻る（ドロップ効果）

### 連続フィルモード
- Fill Intensityを100%に設定すると連続フィル
- 強度を下げるまでフィルパターンが継続

---

## CV出力設定

DC結合オーディオインターフェース用（例：Expert Sleepers ES-8）

1. **Settings** ボタンをクリック
2. **Audio Device** タブでマルチチャンネルインターフェースを選択
3. **CV Routing** タブに切り替え
4. 各CV信号に出力チャンネルを割り当て

### CV信号タイプ

| タイプ | 説明 |
|--------|------|
| **Trig** | 1msトリガーパルス (0-1V) |
| **Pitch** | ピッチCV、1V/Oct規格 |
| **Vel** | ベロシティCV (0-1V) |

### 推奨チャンネル配置

- チャンネル 0-1: ステレオオーディオ出力
- チャンネル 2+: CV出力

---

## システム要件

### macOS
- macOS 10.15 (Catalina) 以降
- Apple Silicon または Intel プロセッサ

### Windows
- Windows 10 以降
- 64-bit プロセッサ

### オーディオインターフェース
- Core Audio (macOS) または ASIO (Windows) 対応インターフェース
- CV出力にはDC結合インターフェースが必要

---

## トラブルシューティング

### 音が出ない
1. Settings → Audio Device を開く
2. 正しい出力デバイスが選択されているか確認
3. サンプルレートがインターフェースと一致しているか確認

### CV出力がない
1. オーディオインターフェースがマルチチャンネル出力に対応しているか確認
2. CV Routingタブで正しいチャンネルを設定
3. チャンネル0-1はオーディオ用、CVはチャンネル2から

### macOS でアプリが開けない
右クリック →「開く」、または「システム設定」→「プライバシーとセキュリティ」で許可

---

## お問い合わせ

- GitHub: https://github.com/mmmmmmmadman/Techno_Machine
- Issues: https://github.com/mmmmmmmadman/Techno_Machine/issues
