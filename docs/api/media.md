---
layout: default
title: メディアAPI
---

# メディア API リファレンス

音声・動画再生に関するAPIです。

## 目次

- [HSP互換関数](#hsp互換関数)
- [Mediaクラス（OOP版）](#mediaクラスoop版)

---

## HSP互換関数

### mmload

メディアファイルを読み込みます。

```cpp
int mmload(std::string_view filename, OptInt bufferId = {}, OptInt mode = {});
```

| パラメータ | 説明 |
|-----------|------|
| `filename` | ファイル名 |
| `bufferId` | バッファID（省略時: 0） |
| `mode` | 読み込みモード |

対応形式: WAV, MP3, OGG, WMA, MP4, AVI 等

**戻り値:** 成功時0、失敗時負の値

**使用例:**

```cpp
mmload("bgm.mp3", 0);      // BGM
mmload("se_shot.wav", 1);  // 効果音
```

---

### mmplay

メディアを再生します。

```cpp
int mmplay(OptInt bufferId = {});
```

**使用例:**

```cpp
mmload("bgm.mp3", 0);
mmplay(0);  // 再生開始
```

---

### mmstop

メディアを停止します。

```cpp
void mmstop(OptInt bufferId = {});
```

| bufferId | 説明 |
|----------|------|
| 0以上 | 指定バッファを停止 |
| -1 | すべてのメディアを停止 |

**使用例:**

```cpp
mmstop(0);   // バッファ0を停止
mmstop(-1);  // すべて停止
```

---

### mmvol

音量を設定します。

```cpp
void mmvol(int bufferId, int vol);
```

| パラメータ | 範囲 | 説明 |
|-----------|------|------|
| `bufferId` | 0以上 | バッファID |
| `vol` | -1000〜0 | 音量（0=最大、-1000=消音） |

**使用例:**

```cpp
mmvol(0, 0);      // 最大音量
mmvol(0, -500);   // 50%音量
mmvol(0, -1000);  // 消音
```

---

### mmpan

パン（左右バランス）を設定します。

```cpp
void mmpan(int bufferId, int pan);
```

| パラメータ | 範囲 | 説明 |
|-----------|------|------|
| `bufferId` | 0以上 | バッファID |
| `pan` | -1000〜1000 | -1000=左、0=中央、1000=右 |

**使用例:**

```cpp
mmpan(0, 0);      // 中央
mmpan(0, -500);   // やや左
mmpan(0, 1000);   // 右端
```

---

### mmstat

メディアの状態を取得します。

```cpp
int mmstat(int bufferId, OptInt mode = {});
```

| mode | 戻り値 |
|------|--------|
| 0 | 再生状態（0=停止, 1=再生中, 2=一時停止） |
| 1 | 再生位置（ミリ秒） |
| 2 | 総時間（ミリ秒） |

**使用例:**

```cpp
if (mmstat(0, 0) == 1) {
    logmes("Playing...");
}

int pos = mmstat(0, 1);
int total = mmstat(0, 2);
logmes(format("Position: {} / {} ms", pos, total));
```

---

## Mediaクラス（OOP版）

### 基本使用法

```cpp
class Media {
public:
    Media();
    explicit Media(std::string_view filename);
    ~Media();
    
    // ムーブ可能、コピー不可
    Media(Media&& other) noexcept;
    Media& operator=(Media&& other) noexcept;
    
    // ファイル操作
    bool load(std::string_view filename);
    void unload();
    
    // 再生制御
    bool play();
    void stop();
    
    // 設定（メソッドチェーン対応）
    Media& vol(int v);     // 音量設定
    Media& pan(int p);     // パン設定
    Media& loop(bool l);   // ループ設定
    Media& mode(int m);    // 再生モード設定
    Media& target(int screenId);  // 動画再生先指定
    
    // 状態取得
    [[nodiscard]] int get_vol() const;
    [[nodiscard]] int get_pan() const;
    [[nodiscard]] bool get_loop() const;
    [[nodiscard]] int get_mode() const;
    [[nodiscard]] int stat() const;
    [[nodiscard]] bool playing() const;
    [[nodiscard]] bool loaded() const;
    [[nodiscard]] const std::string& filename() const;
    [[nodiscard]] int id() const;
};
```

---

### コンストラクタと初期化

```cpp
// デフォルトコンストラクタ（空）
Media bgm;
bgm.load("bgm.mp3");

// ファイル名で直接初期化
Media se("shot.wav");
```

---

### メソッドチェーン

設定メソッドは `Media&` を返すため、メソッドチェーンが可能です。

```cpp
Media bgm("bgm.mp3");
bgm.vol(-200)     // 音量80%
   .loop(true)    // ループ再生
   .play();       // 再生開始
```

---

### 使用例

#### BGM再生

```cpp
Media bgm;

void hspMain() {
    screen(0, 640, 480);
    
    bgm.load("bgm.mp3");
    bgm.vol(-300).loop(true).play();
    
    // ゲームループ
    while (true) {
        redraw(0);
        // ... 描画処理 ...
        redraw(1);
        await(16);
    }
    
    return 0;
}
```

#### 効果音

```cpp
Media se_shot("shot.wav");
Media se_explosion("explosion.wav");

void playShot() {
    se_shot.play();
}

void playExplosion() {
    se_explosion.vol(-500).play();  // 小さめの音量で再生
}
```

#### 動画再生

```cpp
Media video;

void hspMain() {
    screen(0, 640, 480);
    
    video.load("intro.mp4");
    video.target(0);  // ウィンドウ0に表示
    video.play();
    
    // 動画終了待ち
    while (video.playing()) {
        await(100);
    }
    
    // 動画終了後の処理...
    return 0;
}
```

#### フェードアウト

```cpp
void fadeOutBgm(Media& bgm, int durationMs) {
    int startVol = bgm.get_vol();
    int steps = durationMs / 16;
    int volStep = (startVol - (-1000)) / steps;
    
    for (int i = 0; i < steps; i++) {
        int newVol = startVol + ((-1000 - startVol) * i / steps);
        bgm.vol(newVol);
        await(16);
    }
    
    bgm.stop();
}
```

---

## 使用例

### サウンド管理クラス

```cpp
class SoundManager {
    Media bgm;
    std::vector<Media> sePool;
    int seIndex = 0;
    
public:
    void playBgm(std::string_view file) {
        bgm.load(file);
        bgm.loop(true).vol(-200).play();
    }
    
    void stopBgm() {
        bgm.stop();
    }
    
    void setBgmVolume(int vol) {
        bgm.vol(vol);
    }
    
    void playSe(std::string_view file) {
        if (sePool.empty()) {
            sePool.resize(8);  // 8チャンネルのSEプール
        }
        
        sePool[seIndex].load(file);
        sePool[seIndex].play();
        seIndex = (seIndex + 1) % sePool.size();
    }
};

SoundManager sound;

void hspMain() {
    screen(0, 640, 480);
    
    sound.playBgm("title.mp3");
    
    onclick([]() {
        sound.playSe("click.wav");
        return 0;
    });
    
    return 0;
}
```

---

## 参照

- [割り込み API](/HSPPP_Lib/api/interrupt)（await, stop）
- [画面制御 API](/HSPPP_Lib/api/screen)（動画表示先）
