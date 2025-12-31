# 型定義リファレンス

HSPPP で使用される型と構造体の定義です。

## 目次

- [省略可能パラメータ](#省略可能パラメータ)
- [ハンドルクラス](#ハンドルクラス)
- [パラメータ構造体](#パラメータ構造体)
- [図形描画用構造体](#図形描画用構造体)
- [その他の型](#その他の型)

---

## 省略可能パラメータ

HSPでは `screen , 800, 600` のように任意の位置でパラメータを省略できます。
HSPPPでは `omit` キーワードまたは `{}` でこれを実現します。

### omit

パラメータの省略を示すプレースホルダーです。

```cpp
inline constexpr detail::OmitTag omit {};
```

**使用例:**

```cpp
// ID を省略して 800x600 のウィンドウを作成
screen(omit, 800, 600);

// サイズを省略して mode だけ指定
screen(1, omit, omit, screen_hide);

// {} でも省略可能
screen({}, 800, 600);
```

---

### OptInt

省略可能な `int` 型です。

```cpp
struct OptInt {
    constexpr OptInt();                    // デフォルト（省略）
    constexpr OptInt(detail::OmitTag);     // omit からの変換
    constexpr OptInt(int v);               // 値の指定
    
    [[nodiscard]] constexpr bool is_default() const noexcept;
    [[nodiscard]] constexpr int value_or(int def) const noexcept;
    [[nodiscard]] constexpr int value() const;
};
```

---

### OptInt64

省略可能な `int64_t` 型です。ファイルサイズなど大きな値に使用します。

```cpp
struct OptInt64 {
    constexpr OptInt64();
    constexpr OptInt64(detail::OmitTag);
    constexpr OptInt64(int64_t v);
    constexpr OptInt64(int v);  // intからの暗黙変換
    
    [[nodiscard]] constexpr bool is_default() const noexcept;
    [[nodiscard]] constexpr int64_t value_or(int64_t def) const noexcept;
    [[nodiscard]] constexpr int64_t value() const;
};
```

---

### OptDouble

省略可能な `double` 型です。

```cpp
struct OptDouble {
    constexpr OptDouble();
    constexpr OptDouble(detail::OmitTag);
    constexpr OptDouble(double v);
    constexpr OptDouble(int v);  // intからの暗黙変換
    
    [[nodiscard]] constexpr bool is_default() const noexcept;
    [[nodiscard]] constexpr double value_or(double def) const noexcept;
    [[nodiscard]] constexpr double value() const;
};
```

---

## ハンドルクラス

### Screen

ウィンドウまたはバッファを表す軽量ハンドルクラスです。
内部はIDのみ保持し、コピーコストは最小限です。

```cpp
class Screen {
public:
    Screen();                           // 無効なハンドル
    explicit Screen(int id);            // IDから構築
    Screen(int id, bool valid);         // 内部用
    
    // 状態確認
    [[nodiscard]] bool valid() const noexcept;
    [[nodiscard]] explicit operator bool() const noexcept;
    [[nodiscard]] int id() const noexcept;
    
    // 描画関数（メソッドチェーン対応）
    Screen& color(int r, int g, int b);
    Screen& pos(int x, int y);
    Screen& mes(std::string_view text, OptInt sw = {});
    Screen& boxf(int x1, int y1, int x2, int y2);
    Screen& boxf();
    Screen& cls(int mode = 0);
    Screen& redraw(int mode = 1);
    Screen& line(int x2, int y2);
    Screen& circle(int x1, int y1, int x2, int y2, int fillMode = 1);
    Screen& pset(int x, int y);
    Screen& pget(int x, int y);
    Screen& gradf(int x, int y, int w, int h, int mode, int color1, int color2);
    Screen& grect(int cx, int cy, double angle, int w, int h);
    
    // 画面制御
    Screen& select();      // gsel相当
    Screen& show();        // 表示
    Screen& hide();        // 非表示
    Screen& activate();    // 最前面
    Screen& title(std::string_view title);
    Screen& width(int clientW, int clientH = -1, int posX = -1, int posY = -1, int option = 0);
    Screen& groll(int scrollX, int scrollY);
    
    // 情報取得
    [[nodiscard]] int width() const;
    [[nodiscard]] int height() const;
    [[nodiscard]] int mousex() const;
    [[nodiscard]] int mousey() const;
    
    // 画像操作
    Screen& picload(std::string_view filename, int mode = 0);
    Screen& bmpsave(std::string_view filename);
    Screen& gmode(int mode, int sizeX = 32, int sizeY = 32, int blendRate = 256);
    Screen& gcopy(int srcId, int srcX, int srcY, OptInt sizeX = {}, OptInt sizeY = {});
    Screen& gzoom(int destW, int destH, int srcId, int srcX, int srcY, OptInt srcW = {}, OptInt srcH = {}, int mode = 0);
    Screen& grotate(int srcId, int srcX, int srcY, double angle, OptInt dstW = {}, OptInt dstH = {});
    Screen& gsquare(int srcId, const Quad& dst);
    Screen& gsquare(int srcId, const Quad& dst, const QuadUV& src);
    Screen& gsquare(int srcId, const Quad& dst, const QuadColors& colors);
    Screen& celput(const Cel& cel, int cellIndex, OptInt x = {}, OptInt y = {});
    
    // フォント
    Screen& font(std::string_view fontName, int size = 12, int style = 0);
    Screen& sysfont(int type = 0);
    
    // 割り込み
    Screen& onclick(InterruptHandler handler);
    Screen& oncmd(InterruptHandler handler, int messageId);
    Screen& onkey(InterruptHandler handler);
    
    // GUIオブジェクト
    int button(std::string_view name, std::function<int()> callback, bool isGosub = true);
    int input(std::shared_ptr<std::string> var, int maxLength = 1024, int mode = 0);
    int mesbox(std::shared_ptr<std::string> var, int maxLength = 4096, int mode = 0);
    int chkbox(std::string_view label, std::shared_ptr<int> var);
    int combox(std::shared_ptr<int> var, int expandY, std::string_view items);
    int listbox(std::shared_ptr<int> var, int expandY, std::string_view items);
    Screen& objsize(int sizeX, int sizeY = 24, int spaceY = 0);
    Screen& objmode(int mode, int tabMove = -1);
    Screen& objcolor(int r, int g, int b);
    Screen& objprm(int objectId, std::string_view value);
    Screen& objprm(int objectId, int value);
    Screen& objenable(int objectId, int enable = 1);
    Screen& objsel(int objectId);
    Screen& mouse(int x, int y);
};
```

**使用例:**

```cpp
// OOP版でメソッドチェーン
auto win = screen({.width = 640, .height = 480});
win.color(255, 0, 0)
   .boxf(0, 0, 100, 100)
   .pos(10, 10)
   .mes("Hello!")
   .redraw();
```

---

### Cel

画像素材を表す軽量ハンドルクラスです。

```cpp
class Cel {
public:
    Cel();                              // 無効なハンドル
    explicit Cel(int id);               // IDから構築
    Cel(int id, bool valid);            // 内部用
    
    [[nodiscard]] bool valid() const noexcept;
    [[nodiscard]] explicit operator bool() const noexcept;
    [[nodiscard]] int id() const noexcept;
    
    // 操作
    Cel& divide(int divX, int divY);
    Cel& put(int cellIndex, OptInt x = {}, OptInt y = {});
    
    // 情報取得
    [[nodiscard]] int width() const;
    [[nodiscard]] int height() const;
};
```

---

## パラメータ構造体

### ScreenParams

`screen` 命令のパラメータ構造体です。Designated Initializers で使用します。

```cpp
struct ScreenParams {
    int width    = 640;     // 画面サイズX
    int height   = 480;     // 画面サイズY
    int mode     = 0;       // 画面モード（screen_* フラグの組み合わせ）
    int pos_x    = -1;      // ウィンドウ位置X（-1=システム規定）
    int pos_y    = -1;      // ウィンドウ位置Y（-1=システム規定）
    int client_w = 0;       // クライアントサイズX（0=widthと同じ）
    int client_h = 0;       // クライアントサイズY（0=heightと同じ）
    std::string_view title = "HSPPP Window";  // ウィンドウタイトル
};
```

**使用例:**

```cpp
auto win = screen({
    .width = 800,
    .height = 600,
    .mode = screen_fixedsize,
    .title = "My Application"
});
```

---

### BufferParams

`buffer` 命令のパラメータ構造体です。

```cpp
struct BufferParams {
    int width    = 640;     // 画面サイズX
    int height   = 480;     // 画面サイズY
    int mode     = 0;       // 画面モード
};
```

---

### BgscrParams

`bgscr` 命令のパラメータ構造体です。

```cpp
struct BgscrParams {
    int width    = 640;
    int height   = 480;
    int mode     = 0;       // 0=フルカラー, 2=非表示
    int pos_x    = -1;
    int pos_y    = -1;
    int client_w = 0;
    int client_h = 0;
    std::string_view title = "HSPPP Window";
};
```

---

## 図形描画用構造体

### Point2i

2次元座標を表す構造体です。

```cpp
struct Point2i {
    int x = 0;
    int y = 0;
    
    constexpr Point2i() noexcept = default;
    constexpr Point2i(int px, int py) noexcept;
};
```

---

### Quad

4頂点座標を表す構造体です。`gsquare` のコピー先に使用します。
頂点順序: 左上, 右上, 右下, 左下（時計回り）

```cpp
struct Quad {
    static constexpr size_t vertex_count = 4;
    Point2i v[vertex_count];
    
    constexpr Quad() noexcept = default;
    constexpr Quad(Point2i p0, Point2i p1, Point2i p2, Point2i p3) noexcept;
    constexpr Quad(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3) noexcept;
    
    // アクセス
    constexpr Point2i& operator[](size_t i);
    constexpr const Point2i& operator[](size_t i) const;
    constexpr Point2i& at(size_t i);
    constexpr const Point2i& at(size_t i) const;
    [[nodiscard]] static constexpr size_t size() noexcept;
    [[nodiscard]] constexpr std::span<Point2i, vertex_count> data() noexcept;
    
    // イテレータ
    constexpr auto begin() noexcept;
    constexpr auto end() noexcept;
};
```

**使用例:**

```cpp
// Point2i で初期化
Quad dst = {{0, 0}, {100, 0}, {100, 100}, {0, 100}};

// 直接座標で初期化
Quad dst2(0, 0, 100, 20, 80, 120, -20, 100);

// 範囲ベースfor
for (auto& p : dst) {
    p.x += 10;
}
```

---

### QuadUV

4頂点UV座標を表す構造体です。`gsquare` のコピー元に使用します。

```cpp
struct QuadUV {
    static constexpr size_t vertex_count = 4;
    Point2i v[vertex_count];
    
    // Quadと同じインターフェース
};
```

---

### QuadColors

4頂点カラーを表す構造体です。`gsquare` のグラデーションに使用します。

```cpp
struct QuadColors {
    static constexpr size_t color_count = 4;
    int colors[color_count];  // 0xRRGGBB形式
    
    constexpr QuadColors() noexcept;
    constexpr QuadColors(int c0, int c1, int c2, int c3) noexcept;
    
    // アクセス
    constexpr int& operator[](size_t i);
    constexpr const int& operator[](size_t i) const;
    [[nodiscard]] static constexpr size_t size() noexcept;
};
```

**使用例:**

```cpp
QuadColors colors = {0xFF0000, 0x00FF00, 0x0000FF, 0xFFFF00};
gsquare(gsquare_grad, dst, colors);
```

---

## その他の型

### DialogResult

`dialog` 命令の結果を表す構造体です。

```cpp
struct DialogResult {
    int stat;           // ステータス値（ボタンID、成功/失敗など）
    std::string refstr; // 文字列結果（ファイルパスなど）
    
    // 暗黙の型変換
    operator int() const noexcept;        // stat を返す
    operator std::string() const;         // refstr を返す
    explicit operator bool() const noexcept;  // stat != 0
};
```

**使用例:**

```cpp
auto result = dialog("ファイルを選択", dialog_open);
if (result) {
    std::string filename = result;  // refstr を取得
    picload(filename);
}
```

---

### InterruptHandler / ErrorHandler

割り込みハンドラの型定義です。

```cpp
// 汎用割り込みハンドラ（onclick, onkey, onexit, oncmd用）
using InterruptHandler = std::function<int()>;

// エラーハンドラ（onerror用）
using ErrorHandler = std::function<int(const HspErrorBase&)>;
```

**使用例:**

```cpp
// ラムダ式で指定
onclick([]() {
    logmes("Clicked!");
    return 0;
});

// エラーハンドラ
onerror([](const HspErrorBase& e) {
    logmes(format("Error: {}", e.message()));
    return 0;  // 処理を継続
});
```

---

## 参照

- [画面制御 API](screen.md)
- [描画 API](drawing.md)
- [割り込み API](interrupt.md)
