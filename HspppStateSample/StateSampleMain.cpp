// ═══════════════════════════════════════════════════════════════════════════
// HspppStateSample - StateMachineを使ったサンプルアプリケーション
// ═══════════════════════════════════════════════════════════════════════════
//
// このサンプルは HSPPP の StateMachine を使って、
// HSP の *label / goto と同等の画面遷移を型安全に実装する方法を示します。
//
// ✅ HSPユーザーへ: 以下の対応を覚えれば大丈夫！
//   HSP:   *title     →  HSPPP: GameScreen::Title
//   HSP:   goto *game →  HSPPP: sm.jump(GameScreen::Game)
//   HSP:   gosub      →  HSPPP: (on_enter/on_exit で代用)
//
// ═══════════════════════════════════════════════════════════════════════════

// Windows仮想キーコード定義
constexpr int KEY_ESCAPE = 27;      // KEY_ESCAPE
constexpr int KEY_SPACE = 32;       // KEY_SPACE
constexpr int KEY_LBUTTON = 1;      // マウス左ボタン

import hsppp;
using namespace hsppp;

// ═══════════════════════════════════════════════════════════════════════════
// ステップ1: 画面（ステート）を enum class で定義
// ═══════════════════════════════════════════════════════════════════════════
// HSPの *label に相当します。
// enum class を使うことで、タイポがコンパイルエラーになります（HSPと同じ安全性）

enum class GameScreen {
    Splash,     // スプラッシュ画面（起動時）
    Title,      // タイトル画面
    HowToPlay,  // 遊び方説明
    Game,       // ゲーム画面
    Pause,      // ポーズ画面
    GameOver,   // ゲームオーバー
    Result      // リザルト画面
};

// ═══════════════════════════════════════════════════════════════════════════
// ゲーム状態（グローバル変数）
// ═══════════════════════════════════════════════════════════════════════════

int g_score = 0;
int g_highScore = 0;
int g_playerX = 320;
int g_playerY = 400;
int g_targetX = 320;
int g_targetY = 100;
int g_targetRadius = 30;
bool g_gameActive = false;

// ═══════════════════════════════════════════════════════════════════════════
// メイン関数
// ═══════════════════════════════════════════════════════════════════════════

void hspMain() {
    // ウィンドウ初期化
    screen(0, 640, 480);
    title("HSPPP StateMachine サンプル - クリックゲーム");
    
    // ═══════════════════════════════════════════════════════════════════════
    // ステップ2: StateMachine を作成
    // ═══════════════════════════════════════════════════════════════════════
    
    StateMachine<GameScreen> sm;
    
    // デバッグログを有効化（開発時に便利）
    sm.enable_debug_log(true);
    
    // 履歴機能を有効化（back() で戻れるように）
    sm.enable_history(5);
    
    // ✅ デバッグ用: ステート名を登録
    sm.set_state_name(GameScreen::Splash, "Splash");
    sm.set_state_name(GameScreen::Title, "Title");
    sm.set_state_name(GameScreen::HowToPlay, "HowToPlay");
    sm.set_state_name(GameScreen::Game, "Game");
    sm.set_state_name(GameScreen::Pause, "Pause");
    sm.set_state_name(GameScreen::GameOver, "GameOver");
    sm.set_state_name(GameScreen::Result, "Result");
    
    // ═══════════════════════════════════════════════════════════════════════
    // ステップ3: 各画面（ステート）の処理を定義
    // ═══════════════════════════════════════════════════════════════════════
    
    // ─────────────────────────────────────────────────────────────
    // スプラッシュ画面: 2秒後にタイトルへ自動遷移
    // ─────────────────────────────────────────────────────────────
    sm.state(GameScreen::Splash)
      .on_enter([&]() {
          // タイマーで2秒後に自動遷移
          sm.set_timer(GameScreen::Title, 2000);
      })
      .on_update([&](StateMachine<GameScreen>&) {
          // ✅ HSP流: on_update内でredraw/awaitを完結
          redraw(0);
          
          // 背景クリア（clsは使わない - オブジェクト破壊を避ける）
          color(50, 50, 80);
          boxf();
          
          color(255, 255, 255);
          pos(220, 200);
          mes("HSPPP StateMachine");
          
          color(200, 200, 200);
          pos(250, 240);
          mes("Loading...");
          
          // ローディングバー風の演出
          double progress = (sm.state_frame_count() * 100) / 120;
          color(100, 100, 100);
          boxf(170, 280, 470, 300);
          color(100, 200, 255);
          boxf(170, 280, 170 + static_cast<int>(progress * 3), 300);
          
          vwait();  // アニメーション画面: 60FPS
      });
    
    // ─────────────────────────────────────────────────────────────
    // タイトル画面: ボタンUIで画面遷移
    // ─────────────────────────────────────────────────────────────
    sm.state(GameScreen::Title)
      .on_enter([&]() {
          // ✅ on_enter: GUIオブジェクトを作成（1回だけ）
          objsize(200, 40);
          pos(220, 250);
          button("ゲームスタート", [&]() { 
              g_score = 0;
              sm.jump(GameScreen::Game); 
          });
          
          pos(220, 300);
          button("遊び方", [&]() { sm.jump(GameScreen::HowToPlay); });
          
          pos(220, 350);
          button("終了", [&]() { sm.quit(); });
      })
      .on_update([&](StateMachine<GameScreen>&) {
          // ✅ GUI画面: イベント駆動でCPU負荷を抑える
          redraw(0);
          
          color(30, 30, 60);
          boxf();
          
          // タイトルロゴ
          color(255, 220, 100);
          pos(180, 100);
          font(msgothic, 48);
          mes("クリックゲーム");
          
          // ハイスコア表示
          font(msgothic, 16);
          color(200, 200, 200);
          pos(250, 180);
          mes(strf("ハイスコア: %d", g_highScore));
          
          // キーボードでも操作可能
          if (getkey(KEY_SPACE)) {
              g_score = 0;
              sm.jump(GameScreen::Game);
          }
          if (getkey(KEY_ESCAPE)) {
              sm.quit();
          }
          
          redraw(1);
          stop();  // GUI画面: イベント待ち
      })
      .on_exit([&]() {
          // ✅ on_exit: GUIオブジェクトを破棄
          clrobj();
          font(msgothic, 16);  // フォントをリセット
      });
    
    // ─────────────────────────────────────────────────────────────
    // 遊び方画面
    // ─────────────────────────────────────────────────────────────
    sm.state(GameScreen::HowToPlay)
      .on_enter([&]() {
          objsize(200, 40);
          pos(220, 400);
          button("戻る", [&]() { sm.back(); });  // ✅ back() で前の画面に戻る
      })
      .on_update([&](StateMachine<GameScreen>&) {
          redraw(0);
          
          color(30, 50, 30);
          boxf();
          
          color(255, 255, 255);
          font(msgothic, 24);
          pos(200, 50);
          mes("遊び方");
          
          font(msgothic, 16);
          color(220, 220, 220);
          pos(100, 120);
          mes("・ターゲット（赤い円）をクリックしてください");
          pos(100, 150);
          mes("・クリックするとスコアが加算されます");
          pos(100, 180);
          mes("・時間内にできるだけ多くクリックしよう！");
          pos(100, 230);
          mes("操作方法:");
          pos(120, 260);
          mes("マウス左クリック: ターゲットを狙う");
          pos(120, 290);
          mes("Escキー: ポーズ");
          
          if (getkey(KEY_ESCAPE)) {
              sm.back();
          }
          
          redraw(1);
          stop();  // GUI画面: イベント待ち
      })
      .on_exit([&]() {
          clrobj();
          font("", 16);
      });
    
    // ─────────────────────────────────────────────────────────────
    // ゲーム画面: メインのゲームロジック
    // ─────────────────────────────────────────────────────────────
    sm.state(GameScreen::Game)
      .on_enter([&]() {
          g_gameActive = true;
          g_targetX = 100 + rnd(440);
          g_targetY = 100 + rnd(280);
          
          // 30秒でゲームオーバー
          sm.set_timer(GameScreen::GameOver, 30000);
      })
      .on_update([&](StateMachine<GameScreen>&) {
          redraw(0);
          
          color(20, 20, 40);
          boxf();
          
          // 残り時間計算（30秒 = 30000ms）
          int remainingMs = 30000 - (sm.state_frame_count() * 16);  // 約60FPS
          int remainingSec = remainingMs / 1000;
          if (remainingSec < 0) remainingSec = 0;
          
          // UI表示
          color(255, 255, 255);
          pos(20, 20);
          mes(strf("スコア: %d", g_score));
          pos(520, 20);
          mes(strf("残り: %d秒", remainingSec));
          
          // ターゲット描画（赤い円）
          color(255, 80, 80);
          circle(g_targetX - g_targetRadius, g_targetY - g_targetRadius,
                 g_targetX + g_targetRadius, g_targetY + g_targetRadius, 1);
          
          // クリック判定
          int mx = mousex();
          int my = mousey();
          
          if (getkey(1)) {  // 左クリック
              int dx = mx - g_targetX;
              int dy = my - g_targetY;
              if (dx * dx + dy * dy <= g_targetRadius * g_targetRadius) {
                  g_score += 10;
                  // ターゲットを新しい位置に移動
                  g_targetX = 50 + rnd(540);
                  g_targetY = 80 + rnd(320);
              }
          }
          
          // カーソル位置表示
          color(100, 255, 100);
          circle(mx - 5, my - 5, mx + 5, my + 5, 0);
          
          // ポーズ
          if (getkey(KEY_ESCAPE)) {
              sm.cancel_timer();  // タイマーを一時停止
              sm.jump(GameScreen::Pause);
          }
          
          vwait();  // ゲーム画面: 60FPS
      })
      .on_exit([&]() {
          g_gameActive = false;
      });
    
    // ─────────────────────────────────────────────────────────────
    // ポーズ画面
    // ─────────────────────────────────────────────────────────────
    sm.state(GameScreen::Pause)
      .on_enter([&]() {
          objsize(200, 40);
          pos(220, 250);
          button("ゲームに戻る", [&]() { sm.jump(GameScreen::Game); });
          
          pos(220, 300);
          button("タイトルに戻る", [&]() { sm.jump(GameScreen::Title); });
      })
      .on_update([&](StateMachine<GameScreen>&) {
          redraw(0);
          
          // 背景を暗くする
          color(0, 0, 0);
          boxf();
          
          color(255, 255, 255);
          font(msgothic, 32);
          pos(260, 150);
          mes("PAUSE");
          
          font(msgothic, 16);
          color(200, 200, 200);
          pos(220, 200);
          mes(strf("現在のスコア: %d", g_score));
          
          redraw(1);
          stop();  // GUI画面: イベント待ち
      })
      .on_exit([&]() {
          clrobj();
          font("", 16);
      });
    
    // ─────────────────────────────────────────────────────────────
    // ゲームオーバー画面
    // ─────────────────────────────────────────────────────────────
    sm.state(GameScreen::GameOver)
      .on_enter([&]() {
          // ハイスコア更新
          if (g_score > g_highScore) {
              g_highScore = g_score;
          }
          
          // 3秒後にリザルトへ
          sm.set_timer(GameScreen::Result, 3000);
      })
      .on_update([&](StateMachine<GameScreen>&) {
          redraw(0);
          
          color(60, 20, 20);
          boxf();
          
          color(255, 100, 100);
          font(msgothic, 48);
          pos(180, 150);
          mes("TIME UP!");
          
          font(msgothic, 24);
          color(255, 255, 255);
          pos(220, 250);
          mes(strf("スコア: %d", g_score));
          
          if (g_score >= g_highScore && g_score > 0) {
              color(255, 220, 100);
              pos(200, 300);
              mes("★ NEW RECORD! ★");
          }
          
          vwait();  // 演出画面: タイマー遷移待ち
      })
      .on_exit([&]() {
          font(msgothic, 16);
      });
    
    // ─────────────────────────────────────────────────────────────
    // リザルト画面
    // ─────────────────────────────────────────────────────────────
    sm.state(GameScreen::Result)
      .on_enter([&]() {
          objsize(200, 40);
          pos(220, 350);
          button("もう一度", [&]() { 
              g_score = 0;
              sm.jump(GameScreen::Game); 
          });
          
          pos(220, 400);
          button("タイトルへ", [&]() { sm.jump(GameScreen::Title); });
      })
      .on_update([&](StateMachine<GameScreen>&) {
          redraw(0);
          
          color(30, 30, 50);
          boxf();
          
          color(255, 255, 255);
          font(msgothic, 32);
          pos(250, 80);
          mes("RESULT");
          
          font(msgothic, 24);
          pos(200, 160);
          mes(strf("今回のスコア: %d", g_score));
          
          color(255, 220, 100);
          pos(200, 210);
          mes(strf("ハイスコア: %d", g_highScore));
          
          // 評価
          font(msgothic, 20);
          color(200, 200, 255);
          pos(220, 280);
          if (g_score >= 200) {
              mes("評価: ★★★ すばらしい！");
          } else if (g_score >= 100) {
              mes("評価: ★★ いい調子！");
          } else if (g_score >= 50) {
              mes("評価: ★ がんばろう！");
          } else {
              mes("評価: もっと練習！");
          }
          
          redraw(1);
          stop();  // GUI画面: イベント待ち
      })
      .on_exit([&]() {
          clrobj();
          font("", 16);
      });
    
    // ═══════════════════════════════════════════════════════════════════════
    // ステップ4: 初期ステートを設定してメインループ開始
    // ═══════════════════════════════════════════════════════════════════════
    
    sm.jump(GameScreen::Splash);  // 最初はスプラッシュ画面
    sm.run();  // 各ステート内でawait/stopするので、quit()まで戻ってこない
    
    // ═══════════════════════════════════════════════════════════════════════
    // 終了時にグラフを出力（開発用）
    // ═══════════════════════════════════════════════════════════════════════
    sm.export_graph("state_graph.dot");

    end();
}
