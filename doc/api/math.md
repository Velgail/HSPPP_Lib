# 数学 API リファレンス

数学関数・乱数・イージングに関するAPIです。

## 目次

- [角度変換](#角度変換)
- [乱数](#乱数)
- [範囲制限](#範囲制限)
- [イージング](#イージング)
- [ソート](#ソート)
- [数学定数](#数学定数)
- [標準数学関数](#標準数学関数)

---

## 角度変換

### deg2rad

度数法（度）をラジアンに変換します。

```cpp
[[nodiscard]] inline constexpr double deg2rad(double degrees) noexcept;
```

**使用例:**

```cpp
double angle = deg2rad(45);   // 0.785398...
grect(100, 100, deg2rad(30), 80, 40);  // 30度回転
```

---

### rad2deg

ラジアンを度数法（度）に変換します。

```cpp
[[nodiscard]] inline constexpr double rad2deg(double radians) noexcept;
```

**使用例:**

```cpp
double degrees = rad2deg(M_PI);   // 180.0
```

---

## 乱数

### rnd

乱数を発生させます。

```cpp
[[nodiscard]] int rnd(int p1);
```

| パラメータ | 説明 |
|-----------|------|
| `p1` | 乱数の範囲（1〜32768） |

**戻り値:** 0から(p1-1)までの乱数

**使用例:**

```cpp
int dice = rnd(6) + 1;      // 1〜6
int x = rnd(640);           // 0〜639
int coin = rnd(2);          // 0 or 1
```

---

### randomize

乱数発生を初期化します。

```cpp
void randomize(OptInt p1 = {});
```

| パラメータ | 説明 |
|-----------|------|
| `p1` | 乱数シード（省略時: 時刻ベース） |

**使用例:**

```cpp
randomize();       // 時刻ベースで初期化（毎回異なる乱数列）
randomize(12345);  // 固定シード（再現可能な乱数列）
```

---

## 範囲制限

### limit

整数を指定範囲内に制限します。

```cpp
[[nodiscard]] int limit(int p1, OptInt p2 = {}, OptInt p3 = {});
```

| パラメータ | 説明 |
|-----------|------|
| `p1` | 制限する値 |
| `p2` | 最小値（省略時: 0） |
| `p3` | 最大値（省略時: 255） |

**使用例:**

```cpp
int x = limit(value, 0, 639);     // 0〜639に制限
int r = limit(color_r, 0, 255);   // RGB値を制限
```

---

### limitf

実数を指定範囲内に制限します。

```cpp
[[nodiscard]] double limitf(double p1, OptDouble p2 = {}, OptDouble p3 = {});
```

**使用例:**

```cpp
double vol = limitf(volume, 0.0, 1.0);
```

---

## イージング

アニメーション用のイージング関数です。

### setease

イージング関数の計算式を設定します。

```cpp
void setease(double p1, double p2, OptInt p3 = {});
```

| パラメータ | 説明 |
|-----------|------|
| `p1` | 開始値 |
| `p2` | 終了値 |
| `p3` | イージングタイプ（ease_* 定数） |

---

### getease

イージング値を整数で取得します。

```cpp
[[nodiscard]] int getease(int p1, OptInt p2 = {});
```

| パラメータ | 説明 |
|-----------|------|
| `p1` | 進行値（0〜4096） |
| `p2` | イージングタイプ（省略時: setease設定値） |

---

### geteasef

イージング値を実数で取得します。

```cpp
[[nodiscard]] double geteasef(double p1, OptDouble p2 = {});
```

| パラメータ | 説明 |
|-----------|------|
| `p1` | 進行値（0.0〜1.0） |
| `p2` | イージングタイプ |

---

### イージング定数

```cpp
inline constexpr int ease_linear         = 0;    // 線形
inline constexpr int ease_quad_in        = 1;    // 2次イーズイン
inline constexpr int ease_quad_out       = 2;    // 2次イーズアウト
inline constexpr int ease_quad_inout     = 3;    // 2次イーズインアウト
inline constexpr int ease_cubic_in       = 4;    // 3次イーズイン
inline constexpr int ease_cubic_out      = 5;    // 3次イーズアウト
inline constexpr int ease_cubic_inout    = 6;    // 3次イーズインアウト
inline constexpr int ease_quartic_in     = 7;    // 4次イーズイン
inline constexpr int ease_quartic_out    = 8;    // 4次イーズアウト
inline constexpr int ease_quartic_inout  = 9;    // 4次イーズインアウト
inline constexpr int ease_bounce_in      = 10;   // バウンスイン
inline constexpr int ease_bounce_out     = 11;   // バウンスアウト
inline constexpr int ease_bounce_inout   = 12;   // バウンスインアウト
inline constexpr int ease_shake_in       = 13;   // シェイクイン
inline constexpr int ease_shake_out      = 14;   // シェイクアウト
inline constexpr int ease_shake_inout    = 15;   // シェイクインアウト
inline constexpr int ease_loop           = 0x100; // ループフラグ
```

**使用例:**

```cpp
// アニメーション例
setease(0, 400, ease_quad_out);

for (int t = 0; t <= 4096; t += 64) {
    int x = getease(t);
    
    redraw(0);
    cls(4);
    color(255, 255, 0);
    circle(x, 200, x + 32, 232, 1);
    redraw(1);
    await(16);
}
```

---

## ソート

### sortval

配列を数値でソートします。

```cpp
// int版
void sortval(std::vector<int>& arr, OptInt order = {});

// double版
void sortval(std::vector<double>& arr, OptInt order = {});
```

| order | 説明 |
|-------|------|
| 0 | 昇順 |
| 1 | 降順 |

---

### sortstr

配列を文字列でソートします。

```cpp
void sortstr(std::vector<std::string>& arr, OptInt order = {});
```

---

### sortnote

メモリノート文字列をソートします。

```cpp
void sortnote(std::string& note, OptInt order = {});
```

---

### sortget

ソート前のインデックスを取得します。

```cpp
[[nodiscard]] int sortget(int index);
```

**使用例:**

```cpp
std::vector<int> scores = {85, 92, 78, 95, 88};
sortval(scores, 1);  // 降順ソート

for (int i = 0; i < scores.size(); i++) {
    int originalIndex = sortget(i);
    logmes(format("Rank {}: Player {} with score {}", i + 1, originalIndex, scores[i]));
}
```

---

## 数学定数

```cpp
inline constexpr double M_PI      = 3.14159265358979323846;  // π
inline constexpr double M_PI_2    = M_PI / 2.0;              // π/2
inline constexpr double M_PI_4    = M_PI / 4.0;              // π/4
inline constexpr double M_1_PI    = 1.0 / M_PI;              // 1/π
inline constexpr double M_2_PI    = 2.0 / M_PI;              // 2/π
inline constexpr double M_E       = 2.71828182845904523536;  // e（自然対数の底）
inline constexpr double M_LOG2E   = 1.44269504088896340736;  // log₂(e)
inline constexpr double M_LOG10E  = 0.434294481903251827651; // log₁₀(e)
inline constexpr double M_LN2     = 0.693147180559945309417; // ln(2)
inline constexpr double M_LN10    = 2.30258509299404568402;  // ln(10)
inline constexpr double M_SQRT2   = 1.41421356237309504880;  // √2
inline constexpr double M_SQRT1_2 = 0.707106781186547524401; // 1/√2
inline constexpr double M_SQRT3   = 1.73205080756887729353;  // √3
inline constexpr double M_SQRTPI  = 1.77245385090551602730;  // √π
```

---

## 標準数学関数

`<cmath>` の関数が再エクスポートされています。

### 基本数学関数

```cpp
using std::abs;        // 絶対値
using std::fabs;       // 絶対値（実数）
using std::fmod;       // 剰余
using std::remainder;  // IEEE剰余
using std::fmax;       // 最大値
using std::fmin;       // 最小値
using std::fdim;       // 正の差
using std::fma;        // 融合積和演算
```

### 指数・対数

```cpp
using std::exp;        // eのx乗
using std::exp2;       // 2のx乗
using std::expm1;      // e^x - 1
using std::log;        // 自然対数
using std::log10;      // 常用対数
using std::log2;       // 2を底とする対数
using std::log1p;      // ln(1+x)
```

### 累乗・平方根

```cpp
using std::pow;        // 累乗
using std::sqrt;       // 平方根
using std::cbrt;       // 立方根
using std::hypot;      // 斜辺 √(x²+y²)
```

### 三角関数（ラジアン）

```cpp
using std::sin;        // 正弦
using std::cos;        // 余弦
using std::tan;        // 正接
using std::asin;       // 逆正弦
using std::acos;       // 逆余弦
using std::atan;       // 逆正接
using std::atan2;      // 2引数逆正接
```

### 双曲線関数

```cpp
using std::sinh;       // 双曲線正弦
using std::cosh;       // 双曲線余弦
using std::tanh;       // 双曲線正接
using std::asinh;      // 逆双曲線正弦
using std::acosh;      // 逆双曲線余弦
using std::atanh;      // 逆双曲線正接
```

### 切り捨て・切り上げ・丸め

```cpp
using std::ceil;       // 切り上げ
using std::floor;      // 切り捨て
using std::trunc;      // ゼロ方向への切り捨て
using std::round;      // 四捨五入
using std::nearbyint;  // 現在の丸めモードで丸め
using std::rint;       // 現在の丸めモードで丸め（例外あり）
```

### 符号・分類

```cpp
using std::copysign;   // 符号コピー
using std::signbit;    // 符号ビット判定
using std::isnan;      // NaN判定
using std::isinf;      // 無限大判定
using std::isfinite;   // 有限判定
using std::isnormal;   // 正規数判定
using std::fpclassify; // 浮動小数点数の分類
```

### 特殊関数

```cpp
using std::erf;        // 誤差関数
using std::erfc;       // 補誤差関数
using std::tgamma;     // ガンマ関数
using std::lgamma;     // 対数ガンマ関数
```

---

## 使用例

### 円運動

```cpp
int hspMain() {
    screen(0, 640, 480);
    double t = 0.0;
    
    while (true) {
        redraw(0);
        cls(4);
        
        int cx = 320 + (int)(cos(t) * 100);
        int cy = 240 + (int)(sin(t) * 100);
        
        color(255, 255, 0);
        circle(cx - 16, cy - 16, cx + 16, cy + 16, 1);
        
        t += deg2rad(3);  // 3度ずつ回転
        
        redraw(1);
        await(16);
    }
    return 0;
}
```

### ランダムな星空

```cpp
int hspMain() {
    screen(0, 640, 480);
    randomize();
    
    cls(4);
    for (int i = 0; i < 200; i++) {
        int x = rnd(640);
        int y = rnd(480);
        int brightness = rnd(200) + 55;
        color(brightness, brightness, brightness);
        pset(x, y);
    }
    
    return 0;
}
```

---

## 参照

- [描画 API](drawing.md)（grect, grotate での角度指定）
- [文字列操作 API](string.md)（toInt, toDouble）
