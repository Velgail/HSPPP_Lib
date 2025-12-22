// HspppSample/DemoDrawMedia.cpp
// ═══════════════════════════════════════════════════════════════════
// HSPPP マルチメディアデモ
// 
// 操作:
//   Q/W/E: メディアタイプ選択 (WAV/MP3/MP4)
//   L: ロード  P: 再生  S: 停止
//   ↑/↓: 音量  ←/→: パン
// ═══════════════════════════════════════════════════════════════════

#include "DemoState.h"
import hsppp;
import <format>;
import <string>;
using namespace hsppp;

// ═══════════════════════════════════════════════════════════════════
// グローバル変数（実体定義）
// ═══════════════════════════════════════════════════════════════════
bool g_mediaLoaded = false;
int g_mediaVolume = 0;        // 0 = 最大、-1000 = 無音
int g_mediaPan = 0;           // -1000 = 左、0 = 中央、1000 = 右
bool g_mediaIsPlaying = false;
int g_lastLoadResult = -999;  // mmloadの結果
int g_lastPlayResult = -999;  // mmplayの結果
int g_mediaType = 0;          // 0=WAV, 1=MP3, 2=MP4

// メディアファイルパス
const char* g_mediaFiles[] = {
    "resources\\nc451520.wav",
    "resources\\Nocturne in E flat major, Op. 9 no. 2.mp3",
    "resources\\veo3-drone.mp4"
};
const char* g_mediaTypeNames[] = { "WAV", "MP3", "MP4" };

// ═══════════════════════════════════════════════════════════════════
// マルチメディアデモ描画
// ═══════════════════════════════════════════════════════════════════

void drawMediaDemo(Screen& win) {
    MediaDemo demo = static_cast<MediaDemo>(g_demoIndex);

    switch (demo) {
        case MediaDemo::AudioPlayback: {
            win.font("MS Gothic", 12, 0);
            int y = 70;
            
            // メディアタイプ選択（横並び・コンパクト）
            win.color(180, 180, 180).pos(20, y);
            win.mes("タイプ選択: [Q]WAV [W]MP3 [E]MP4");
            y += 18;
            
            // 選択中のファイル
            win.color(150, 255, 150).pos(20, y);
            win.mes(std::format("選択: {} - {}", g_mediaTypeNames[g_mediaType], g_mediaFiles[g_mediaType]));
            y += 22;

            // 操作（1行にまとめる）
            win.color(180, 180, 180).pos(20, y);
            win.mes("[L]ロード [P]再生 [S]停止  [↑↓]音量 [←→]パン");
            y += 25;

            // 状態表示（コンパクト）
            win.color(255, 255, 255).pos(20, y);
            std::string loadStatus = g_mediaLoaded ? "OK" : "未";
            std::string playStatus = g_mediaIsPlaying ? "再生中" : "停止";
            win.mes(std::format("状態: {} ({}) / {} ({})", 
                loadStatus, g_lastLoadResult, playStatus, g_lastPlayResult));
            y += 20;

            // 音量・パン（横並び・バー付き）
            win.color(200, 200, 200).pos(20, y);
            win.mes(std::format("Vol:{:4d}", g_mediaVolume));
            
            // 音量バー
            int volBarX = 90;
            int volBarWidth = 100;
            int volFill = static_cast<int>((g_mediaVolume + 1000) / 1000.0 * volBarWidth);
            win.color(40, 40, 40).boxf(volBarX, y, volBarX + volBarWidth, y + 14);
            win.color(80, 180, 80).boxf(volBarX, y, volBarX + volFill, y + 14);

            // パン
            win.color(200, 200, 200).pos(210, y);
            win.mes(std::format("Pan:{:5d}", g_mediaPan));
            
            // パンバー
            int panBarX = 290;
            int panBarWidth = 100;
            int panCenter = panBarX + panBarWidth / 2;
            int panFill = static_cast<int>(g_mediaPan / 1000.0 * (panBarWidth / 2));
            win.color(40, 40, 40).boxf(panBarX, y, panBarX + panBarWidth, y + 14);
            win.color(80, 120, 200);
            if (panFill < 0) {
                win.boxf(panCenter + panFill, y, panCenter, y + 14);
            } else {
                win.boxf(panCenter, y, panCenter + panFill, y + 14);
            }
            win.color(255, 255, 0).boxf(panCenter - 1, y, panCenter + 1, y + 14);
            y += 22;

            // アクションログ
            if (!g_actionLog.empty()) {
                win.color(200, 180, 100).pos(20, y);
                win.mes(g_actionLog);
            }
            break;
        }
        default:
            win.color(255, 0, 0).pos(20, 70);
            win.mes("Unknown media demo");
            break;
    }
}

// ═══════════════════════════════════════════════════════════════════
// マルチメディアデモアクション処理
// ═══════════════════════════════════════════════════════════════════

void processMediaAction(Screen& win) {
    // 修飾キーが押されている場合はスキップ
    if (isModifierKeyPressed()) return;

    MediaDemo demo = static_cast<MediaDemo>(g_demoIndex);

    switch (demo) {
        case MediaDemo::AudioPlayback: {
            // Q/W/E: メディアタイプ選択
            static bool prevQ = false, prevW = false, prevE = false;
            bool currQ = getkey('Q') != 0;
            bool currW = getkey('W') != 0;
            bool currE = getkey('E') != 0;
            if (currQ && !prevQ) {
                g_mediaType = 0;
                g_mediaLoaded = false;
                g_lastLoadResult = -999;
                g_lastPlayResult = -999;
                g_actionLog = "WAV選択";
            }
            if (currW && !prevW) {
                g_mediaType = 1;
                g_mediaLoaded = false;
                g_lastLoadResult = -999;
                g_lastPlayResult = -999;
                g_actionLog = "MP3選択";
            }
            if (currE && !prevE) {
                g_mediaType = 2;
                g_mediaLoaded = false;
                g_lastLoadResult = -999;
                g_lastPlayResult = -999;
                g_actionLog = "MP4選択";
            }
            prevQ = currQ; prevW = currW; prevE = currE;

            // L: ロード
            static bool prevL = false;
            bool currL = getkey('L') != 0;
            if (currL && !prevL) {
                std::string filePath = g_mediaFiles[g_mediaType];
                g_lastLoadResult = mmload(filePath, 0, 0);
                if (g_lastLoadResult == 0) {
                    g_mediaLoaded = true;
                    g_actionLog = "Loaded OK";
                } else {
                    g_mediaLoaded = false;
                    g_actionLog = std::format("Load failed ({})", g_lastLoadResult);
                }
            }
            prevL = currL;

            // P: 再生
            static bool prevP = false;
            bool currP = getkey('P') != 0;
            if (currP && !prevP) {
                if (g_mediaLoaded) {
                    g_lastPlayResult = mmplay(0);
                    g_actionLog = (g_lastPlayResult == 0) ? "Playing" : std::format("Play failed ({})", g_lastPlayResult);
                    if (g_lastPlayResult == 0) g_mediaIsPlaying = true;
                } else {
                    g_actionLog = "Load first (L)";
                }
            }
            prevP = currP;

            // S: 停止
            static bool prevS = false;
            bool currS = getkey('S') != 0;
            if (currS && !prevS) {
                mmstop(0);
                g_mediaIsPlaying = false;
                g_actionLog = "Stopped";
            }
            prevS = currS;

            // ↑↓: 音量
            if (getkey(VK::UP)) {
                g_mediaVolume = std::min(0, g_mediaVolume + 50);
                mmvol(0, g_mediaVolume);
            }
            if (getkey(VK::DOWN)) {
                g_mediaVolume = std::max(-1000, g_mediaVolume - 50);
                mmvol(0, g_mediaVolume);
            }

            // ←→: パン
            if (getkey(VK::RIGHT)) {
                g_mediaPan = std::min(1000, g_mediaPan + 50);
                mmpan(0, g_mediaPan);
            }
            if (getkey(VK::LEFT)) {
                g_mediaPan = std::max(-1000, g_mediaPan - 50);
                mmpan(0, g_mediaPan);
            }

            // 再生状態を更新
            if (g_mediaLoaded) {
                int stat = mmstat(0, 16);
                g_mediaIsPlaying = (stat != 0);
            }
            break;
        }
        default:
            break;
    }
}
