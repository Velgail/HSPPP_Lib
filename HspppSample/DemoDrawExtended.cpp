// HspppSample/DemoDrawExtended.cpp
// ═══════════════════════════════════════════════════════════════════
// HSPPP デモアプリケーション - 拡張デモ描画
// ═══════════════════════════════════════════════════════════════════

#include "DemoState.h"
import hsppp;
import <format>;
import <vector>;
import <string>;
using namespace hsppp;

// ソートデモ用の定数と共有データ
namespace {
    // 初期データ（定数として一元管理）
    const std::vector<int> SORT_INIT_INT = { 64, 34, 25, 12, 22, 11, 90, 45 };
    const std::vector<std::string> SORT_INIT_STR = { "Banana", "Apple", "Cherry", "Date", "Elderberry" };
    const std::string SORT_INIT_NOTE = "Zebra\nApple\nMango\nBanana\nCherry";

    // 共有状態（drawとactionで共有）
    std::vector<int> g_sortIntArr = SORT_INIT_INT;
    std::vector<std::string> g_sortStrArr = SORT_INIT_STR;
    std::string g_sortNoteData = SORT_INIT_NOTE;
    bool g_sortDone = false;
    std::vector<int> g_sortOrigIndices;

    // ソートデータをリセット
    void resetSortData() {
        g_sortIntArr = SORT_INIT_INT;
        g_sortStrArr = SORT_INIT_STR;
        g_sortNoteData = SORT_INIT_NOTE;
        g_sortDone = false;
        g_sortOrigIndices.clear();
    }
}

void drawExtendedDemo(Screen& win) {
    switch (static_cast<ExtendedDemo>(g_demoIndex)) {
    case ExtendedDemo::Math:
        win.color(0, 0, 0).pos(20, 85);
        win.mes("数学関数デモ: sin, cos, rnd, limit, sqrt, pow");
        
        // Sin/Cos波形描画
        win.color(0, 128, 0).pos(50, 120);
        win.mes("sin/cos 波形 (角度を自動更新中)");
        
        // 波形の背景
        win.color(240, 240, 240);
        win.boxf(50, 150, 590, 250);
        
        // X軸
        win.color(128, 128, 128);
        win.line(590, 200, 50, 200);
        
        // Sin波形（赤）
        win.color(255, 0, 0);
        for (int x = 0; x < 540; x++) {
            double angle = hsppp::deg2rad(g_angle + x * 2);
            int y = 200 - static_cast<int>(hsppp::sin(angle) * 40);
            if (x == 0) {
                win.pos(50 + x, y);
            } else {
                win.line(50 + x, y);
            }
        }
        
        // Cos波形（青）
        win.color(0, 0, 255);
        for (int x = 0; x < 540; x++) {
            double angle = hsppp::deg2rad(g_angle + x * 2);
            int y = 200 - static_cast<int>(hsppp::cos(angle) * 40);
            if (x == 0) {
                win.pos(50 + x, y);
            } else {
                win.line(50 + x, y);
            }
        }
        
        // 乱数表示
        win.font("MS Gothic", 12, 0);
        win.color(0, 0, 0).pos(50, 270);
        win.mes("rnd(100) の結果:");
        for (int i = 0; i < 10; i++) {
            win.pos(50 + i * 50, 290);
            win.mes(str(rnd(100)));
        }
        
        // limit表示
        win.pos(50, 320);
        win.mes("limit デモ:");
        win.pos(50, 340);
        win.mes("limit(-50, 0, 100) = " + str(hsppp::limit(-50, 0, 100)));
        win.pos(50, 355);
        win.mes("limit(150, 0, 100) = " + str(hsppp::limit(150, 0, 100)));
        
        // sqrt/pow表示
        win.pos(300, 320);
        win.mes("sqrt/pow デモ:");
        win.pos(300, 340);
        win.mes("sqrt(2) = " + str(hsppp::sqrt(2.0)));
        win.pos(300, 355);
        win.mes("pow(2, 10) = " + str(hsppp::pow(2.0, 10.0)));
        
        // 角度を更新（アニメーション）
        g_angle += 2.0;
        if (g_angle >= 360.0) g_angle -= 360.0;
        break;
        
    case ExtendedDemo::Color:
        win.color(0, 0, 0).pos(20, 85);
        win.mes("色関連関数デモ: hsvcolor, rgbcolor, syscolor");
        
        // HSVカラーグラデーション
        win.color(0, 0, 0).pos(50, 120);
        win.mes("hsvcolor グラデーション (H: 0-191):");
        for (int h = 0; h < 192; h++) {
            hsvcolor(h, 255, 255);
            win.boxf(50 + h * 2, 140, 50 + h * 2 + 2, 180);
        }
        
        // 彩度グラデーション
        win.color(0, 0, 0).pos(50, 190);
        win.mes("hsvcolor 彩度グラデーション (S: 0-255):");
        for (int s = 0; s < 256; s++) {
            hsvcolor(0, s, 255);
            win.boxf(50 + s * 2, 210, 50 + s * 2 + 2, 250);
        }
        
        // RGBカラー
        win.color(0, 0, 0).pos(50, 270);
        win.mes("rgbcolor サンプル:");
        rgbcolor(0xFF0000); win.boxf(50, 290, 100, 340);
        rgbcolor(0x00FF00); win.boxf(110, 290, 160, 340);
        rgbcolor(0x0000FF); win.boxf(170, 290, 220, 340);
        rgbcolor(0xFFFF00); win.boxf(230, 290, 280, 340);
        rgbcolor(0xFF00FF); win.boxf(290, 290, 340, 340);
        rgbcolor(0x00FFFF); win.boxf(350, 290, 400, 340);
        
        // システムカラー
        win.color(0, 0, 0).pos(50, 360);
        win.mes("syscolor サンプル (システムカラー):");
        for (int i = 0; i < 8; i++) {
            syscolor(i);
            win.boxf(50 + i * 60, 380, 100 + i * 60, 420);
            win.color(0, 0, 0).pos(50 + i * 60, 425);
            win.mes(str(i));
        }
        break;
        
    case ExtendedDemo::Gradf:
        win.color(0, 0, 0).pos(20, 85);
        win.mes("gradf: 矩形をグラデーションで塗りつぶす");
        
        // 横方向グラデーション
        win.color(0, 0, 0).pos(50, 120);
        win.mes("横方向グラデーション (mode=0):");
        gradf(50, 140, 200, 60, 0, 0xFF0000, 0x0000FF);
        gradf(50, 210, 200, 60, 0, 0x00FF00, 0xFFFF00);
        
        // 縦方向グラデーション
        win.color(0, 0, 0).pos(300, 120);
        win.mes("縦方向グラデーション (mode=1):");
        gradf(300, 140, 200, 60, 1, 0xFF00FF, 0x00FFFF);
        gradf(300, 210, 200, 60, 1, 0xFFFFFF, 0x000000);
        
        // OOP版グラデーション
        win.color(0, 0, 0).pos(50, 290);
        win.mes("Screen OOP版 gradf:");
        win.gradf(50, 310, 450, 80, 0, 0xFF8800, 0x0088FF);
        
        // gettime デモ
        win.color(0, 0, 0).pos(50, 410);
        win.mes("gettime 関数:");
        win.pos(50, 430);
        win.mes(std::format("現在時刻: {:04d}/{:02d}/{:02d} {:02d}:{:02d}:{:02d}",
            gettime(0), gettime(1), gettime(3),
            gettime(4), gettime(5), gettime(6)));
        break;
        
    case ExtendedDemo::Grect:
        win.color(0, 0, 0).pos(20, 85);
        win.mes("grect: 回転する矩形で塗りつぶす (← / → で回転)");
        
        // 回転矩形デモ
        win.color(255, 0, 0);
        grect(200, 250, g_angle * 0.0174533, 100, 60);
        
        win.color(0, 255, 0);
        grect(350, 250, g_angle * 0.0174533 + 1.0, 80, 80);
        
        win.color(0, 0, 255);
        grect(500, 250, -g_angle * 0.0174533, 120, 40);
        
        // OOP版
        win.color(255, 128, 0);
        win.grect(320, 380, g_angle * 0.0174533 * 2, 60, 60);
        
        win.color(0, 0, 0).pos(50, 420);
        win.mes("角度: " + str(static_cast<int>(g_angle)) + "度");
        
        // 自動回転
        g_angle += 1.0;
        if (g_angle >= 360.0) g_angle -= 360.0;
        break;
        
    case ExtendedDemo::Gsquare:
        win.color(0, 0, 0).pos(20, 85);
        win.mes("gsquare: 任意の四角形を描画");
        
        // 単色塗りつぶし
        win.color(255, 0, 0);
        {
            Quad solidQuad = {{50, 150}, {200, 150}, {220, 280}, {30, 280}};
            gsquare(-1, solidQuad);
        }
        win.color(0, 0, 0).pos(50, 290);
        win.mes("単色 (srcId=-1)");
        
        // グラデーション
        {
            Quad gradQuad = {{250, 150}, {400, 150}, {420, 280}, {230, 280}};
            QuadColors gradColors = {0xFF0000, 0x00FF00, 0x0000FF, 0xFFFF00};
            gsquare(gsquare_grad, gradQuad, gradColors);
        }
        win.color(0, 0, 0).pos(250, 290);
        win.mes("グラデーション (srcId=-257)");
        
        // 台形
        win.color(0, 128, 255);
        {
            Quad trapezoid = {{500, 200}, {580, 200}, {600, 280}, {480, 280}};
            gsquare(-1, trapezoid);
        }
        win.color(0, 0, 0).pos(480, 290);
        win.mes("台形");
        break;
        
    case ExtendedDemo::Gcopy:
        win.color(0, 0, 0).pos(20, 85);
        win.mes("gcopy: 画面コピー");
        
        // ソースバッファがなければ作成（固定ID 100を使用）
        if (g_srcBufferId.is_default()) {
            [[maybe_unused]] const auto bufferId = buffer(100, 100, 100);  // 固定ID 100でバッファ作成
            g_srcBufferId = 100;
            gsel(g_srcBufferId);
            color(255, 128, 0); boxf();
            color(0, 128, 255); circle(10, 10, 90, 90, 1);
            color(255, 255, 255); pos(25, 40);
            mes("SRC");
            win.select();
        }
        
        // ソースバッファの内容を表示
        win.pos(50, 150);
        gmode(0);
        gcopy(g_srcBufferId, 0, 0, 100, 100);
        win.color(0, 0, 0).pos(50, 260);
        win.mes("コピー元バッファ");
        
        // gcopyで複数箇所にコピー
        win.pos(200, 150);
        gmode(0);
        gcopy(g_srcBufferId, 0, 0, 100, 100);
        win.color(0, 0, 0).pos(200, 260);
        win.mes("gmode(0) 通常");
        
        win.pos(350, 150);
        gmode(2, 100, 100, 128);
        gcopy(g_srcBufferId, 0, 0, 100, 100);
        win.color(0, 0, 0).pos(350, 260);
        win.mes("gmode(2) 半透明");
        
        win.pos(500, 150);
        gmode(5, 100, 100, 200);
        gcopy(g_srcBufferId, 0, 0, 100, 100);
        win.color(0, 0, 0).pos(500, 260);
        win.mes("gmode(5) 加算");
        
        // gmodeを戻す
        gmode(0);
        break;
        
    case ExtendedDemo::Gzoom:
        win.color(0, 0, 0).pos(20, 85);
        win.mes("gzoom: 変倍して画面コピー");
        
        // ソースバッファがなければ作成（gcopyと同じバッファを共有）
        if (g_srcBufferId.is_default()) {
            [[maybe_unused]] const auto bufferId = buffer(100, 100, 100);
            g_srcBufferId = 100;
            gsel(g_srcBufferId);
            color(255, 0, 0); boxf();
            color(0, 255, 0); circle(10, 10, 90, 90, 1);
            color(255, 255, 255); pos(25, 40);
            mes("SRC");
            win.select();
        }
        
        // ソースバッファの内容を表示
        win.pos(50, 120);
        gmode(0);
        gcopy(g_srcBufferId, 0, 0, 50, 50);
        win.color(0, 0, 0).pos(50, 175);
        win.mes("元画像 50x50");
        
        // 拡大コピー
        win.pos(150, 120);
        gzoom(100, 100, g_srcBufferId, 0, 0, 50, 50, 0);
        win.color(0, 0, 0).pos(150, 225);
        win.mes("2倍拡大 (mode=0)");
        
        // 縮小コピー
        win.pos(300, 145);
        gzoom(25, 25, g_srcBufferId, 0, 0, 50, 50, 0);
        win.color(0, 0, 0).pos(300, 175);
        win.mes("0.5倍縮小");
        
        // 高品質拡大
        win.pos(400, 120);
        gzoom(150, 100, g_srcBufferId, 0, 0, 50, 50, 1);
        win.color(0, 0, 0).pos(400, 225);
        win.mes("3x2倍 (mode=1 高品質)");
        
        // 説明
        win.color(0, 128, 0).pos(50, 300);
        win.mes("gzoom(dest_w, dest_h, src_id, src_x, src_y, src_w, src_h, mode)");
        win.pos(50, 320);
        win.mes("mode: 0=高速, 1=高品質ハーフトーン");
        break;
        
    case ExtendedDemo::Grotate:
        win.color(0, 0, 0).pos(20, 85);
        win.mes("grotate: 矩形画像を回転してコピー (←→で角度調整)");
        
        // ソースバッファがなければ作成（gcopy/gzoomと同じバッファを共有）
        if (g_srcBufferId.is_default()) {
            [[maybe_unused]] const auto bufferId = buffer(100, 100, 100);
            g_srcBufferId = 100;
            gsel(g_srcBufferId);
            color(255, 200, 0); boxf();
            color(0, 0, 128); boxf(10, 10, 90, 90);
            color(255, 255, 255); pos(25, 40);
            mes("SRC");
            win.select();
        }
        
        // ソースバッファの内容を表示
        win.pos(50, 120);
        gmode(0);
        gcopy(g_srcBufferId, 0, 0, 60, 60);
        win.color(0, 0, 0).pos(50, 185);
        win.mes("元画像 60x60");
        
        // 回転コピー（中央・回転中）
        win.pos(300, 280);
        grotate(g_srcBufferId, 0, 0, deg2rad(g_angle), 60, 60);
        
        // 固定角度で表示
        win.pos(180, 280);
        grotate(g_srcBufferId, 0, 0, deg2rad(0), 60, 60);
        win.color(0, 0, 0).pos(165, 320);
        win.mes("0度");
        
        win.pos(420, 280);
        grotate(g_srcBufferId, 0, 0, deg2rad(45), 60, 60);
        win.color(0, 0, 0).pos(405, 320);
        win.mes("45度");
        
        win.pos(540, 280);
        grotate(g_srcBufferId, 0, 0, deg2rad(90), 60, 60);
        win.color(0, 0, 0).pos(525, 320);
        win.mes("90度");
        
        // 現在の回転角度
        win.font("MS Gothic", 14, 1);
        win.color(255, 0, 0).pos(280, 340);
        win.mes("回転中: " + str(static_cast<int>(g_angle)) + "度");
        win.font("MS Gothic", 12, 0);
        
        // 自動回転
        g_angle += 2.0;
        if (g_angle >= 360.0) g_angle -= 360.0;
        
        // 説明
        win.color(0, 128, 0).pos(50, 400);
        win.mes("grotate(srcId, srcX, srcY, angle, dstW, dstH)");
        win.pos(50, 420);
        win.mes("角度はラジアン単位 - deg2rad()で変換");
        break;
        
    case ExtendedDemo::StringFunc:
        win.color(0, 0, 0).pos(20, 85);
        win.mes("文字列操作関数: instr, strmid, split, strrep, getstr");
        
        // instr デモ
        win.font("MS Gothic", 12, 1);
        win.color(0, 0, 128).pos(50, 115);
        win.mes("instr - 文字列の検索:");
        win.font("MS Gothic", 12, 0);
        win.color(0, 0, 0).pos(50, 132);
        {
            std::string text = "Hello World, Hello HSP";
            win.mes("検索対象: \"" + text + "\"");
            win.pos(50, 147);
            win.mes("instr(text, \"World\") = " + str(instr(text, "World")));
            win.pos(50, 162);
            win.mes("instr(text, 7, \"Hello\") = " + str(instr(text, 7, "Hello")));
        }
        
        // strmid デモ
        win.font("MS Gothic", 12, 1);
        win.color(0, 128, 0).pos(350, 115);
        win.mes("strmid - 文字列の一部を取り出す:");
        win.font("MS Gothic", 12, 0);
        win.color(0, 0, 0).pos(350, 132);
        {
            std::string text = "ABCDEFGHIJ";
            win.mes("元文字列: \"" + text + "\"");
            win.pos(350, 147);
            win.mes("strmid(text, 2, 3) = \"" + strmid(text, 2, 3) + "\"");
            win.pos(350, 162);
            win.mes("strmid(text, -1, 3) = \"" + strmid(text, -1, 3) + "\"");
        }

        // split デモ（新規追加）
        win.font("MS Gothic", 12, 1);
        win.color(128, 0, 0).pos(50, 190);
        win.mes("split - 文字列を分割:");
        win.font("MS Gothic", 12, 0);
        win.color(0, 0, 0).pos(50, 207);
        {
            std::string csv = "12,34,56,78";
            std::vector<std::string> parts = split(csv, ",");
            win.mes("split(\"" + csv + "\", \",\") = ");
            win.pos(50, 222);
            std::string result = "  結果: [";
            for (size_t i = 0; i < parts.size(); ++i) {
                if (i > 0) result += ", ";
                result += "\"" + parts[i] + "\"";
            }
            result += "] (" + str(static_cast<int>(parts.size())) + "要素)";
            win.mes(result);
        }

        // strrep デモ（新規追加）
        win.font("MS Gothic", 12, 1);
        win.color(0, 128, 0).pos(350, 190);
        win.mes("strrep - 文字列の置換:");
        win.font("MS Gothic", 12, 0);
        win.color(0, 0, 0).pos(350, 207);
        {
            std::string text = "AAA BBB AAA CCC";
            win.mes("元文字列: \"" + text + "\"");
            int64_t count = strrep(text, "AAA", "XXX");
            win.pos(350, 222);
            win.mes("strrep(text, \"AAA\", \"XXX\") = " + str(static_cast<int>(count)) + "回");
            win.pos(350, 237);
            win.mes("  結果: \"" + text + "\"");
        }

        // getstr デモ（新規追加）
        win.font("MS Gothic", 12, 1);
        win.color(128, 0, 128).pos(50, 265);
        win.mes("getstr - バッファから文字列読み出し:");
        win.font("MS Gothic", 12, 0);
        win.color(0, 0, 0).pos(50, 282);
        {
            std::string buf = "ABC,DEF,GHI";
            std::string dest;
            int64_t len1 = getstr(dest, buf, 0, ',');
            win.mes("getstr(dest, \"ABC,DEF,GHI\", 0, ',') = \"" + dest + "\" (len=" + str(static_cast<int>(len1)) + ")");
            win.pos(50, 297);
            int64_t len2 = getstr(dest, buf, len1, ',');
            win.mes("getstr(dest, buf, " + str(static_cast<int>(len1)) + ", ',') = \"" + dest + "\" (len=" + str(static_cast<int>(len2)) + ")");
        }
        
        // strtrim デモ
        win.font("MS Gothic", 12, 1);
        win.color(0, 0, 128).pos(350, 265);
        win.mes("strtrim - 指定文字を除去:");
        win.font("MS Gothic", 12, 0);
        win.color(0, 0, 0).pos(350, 282);
        {
            std::string text = "  Hello World  ";
            win.mes("strtrim(\"" + text + "\", 0) = \"" + strtrim(text, 0, ' ') + "\"");
            win.pos(350, 297);
            win.mes("strtrim(text, 3) = \"" + strtrim(text, 3, ' ') + "\" (全除去)");
        }
        

        // getpath デモ
        win.font("MS Gothic", 12, 1);
        win.color(0, 128, 128).pos(350, 330);
        win.mes("getpath - パスの一部を取得:");
        win.font("MS Gothic", 12, 0);
        win.color(0, 0, 0).pos(350, 347);
        {
            std::string path = "c:\\folder\\test.bmp";
            win.mes("パス: \"" + path + "\"");
            win.pos(350, 362);
            win.mes("getpath(path, 1) = \"" + getpath(path, 1) + "\"");
            win.pos(350, 377);
            win.mes("getpath(path, 8) = \"" + getpath(path, 8) + "\"");
            win.pos(350, 392);
            win.mes("getpath(path, 32) = \"" + getpath(path, 32) + "\"");
        }

        // note系 + sysval デモ
        win.font("MS Gothic", 12, 1);
        win.color(0, 0, 128).pos(50, 330);
        win.mes("note* / sysval(hwnd,hinstance):");
        win.font("MS Gothic", 12, 0);
        win.color(0, 0, 0).pos(50, 347);
        {
            std::string note;
            std::string line;
            notesel(note);
            noteadd("A");
            noteadd("C");
            noteadd("B", 1);
            noteget(line, 1);
            int found = notefind("B", notefind_match);
            noteunsel();

            win.mes("note = \"" + note + "\"");
            win.pos(50, 362);
            win.mes("noteget(idx=1) = \"" + line + "\"");
            win.pos(50, 377);
            win.mes("notefind(\"B\") = " + str(found));
            win.pos(50, 392);
            win.mes("hwnd=" + str(hwnd()) + " hinstance=" + str(hinstance()));
        }

        // 文字列変換関数デモ（cnvstoa, cnvstow, cnvatos, cnvwtos）
        win.font("MS Gothic", 12, 1);
        win.color(128, 0, 128).pos(50, 420);
        win.mes("文字列変換: cnvstow/cnvwtos/cnvstoa/cnvatos");
        win.font("MS Gothic", 12, 0);
        win.color(0, 0, 0).pos(50, 437);
        {
            std::string utf8 = reinterpret_cast<const char*>(u8"日本語ABC");
            // UTF-8 -> UTF-16 -> UTF-8 往復変換
            std::u16string u16 = cnvstow(utf8);
            std::string back1 = cnvwtos(u16);
            win.mes("cnvstow/cnvwtos往復: \"" + utf8 + "\" -> u16(" + str(static_cast<int>(u16.size())) + "文字) -> \"" + back1 + "\"");
            win.pos(50, 452);
            // UTF-8 -> ANSI -> UTF-8 往復変換
            std::string ansi = cnvstoa(utf8);
            std::string back2 = cnvatos(ansi);
            win.mes("cnvstoa/cnvatos往復: \"" + utf8 + "\" -> ANSI(" + str(static_cast<int>(ansi.size())) + "bytes) -> \"" + back2 + "\"");
        }
        
        break;

    case ExtendedDemo::SystemInfo:
        win.color(0, 0, 0).pos(20, 85);
        win.mes("システム情報関数: sysinfo, dirinfo, peek/poke");
        
        // sysinfo デモ
        win.font("MS Gothic", 12, 1);
        win.color(0, 0, 128).pos(50, 115);
        win.mes("sysinfo - システム情報の取得:");
        win.font("MS Gothic", 12, 0);
        win.color(0, 0, 0).pos(50, 135);
        {
            win.mes("sysinfo_str(0) = \"" + sysinfo_str(0) + "\" (OS名)");
            win.pos(50, 153);
            win.mes("sysinfo_str(1) = \"" + sysinfo_str(1) + "\" (ユーザー名)");
            win.pos(50, 171);
            win.mes("sysinfo_str(2) = \"" + sysinfo_str(2) + "\" (コンピュータ名)");
            win.pos(50, 189);
            int64_t cpuNum = sysinfo_int(17);
            win.mes("sysinfo_int(17) = " + std::to_string(cpuNum) + " (CPU数)");
            win.pos(50, 207);
            int64_t totalMem = sysinfo_int(34);
            win.mes("sysinfo_int(34) = " + std::to_string(totalMem) + " (物理メモリMB)");
            win.pos(50, 225);
            int64_t freeMem = sysinfo_int(35);
            win.mes("sysinfo_int(35) = " + std::to_string(freeMem) + " (空きメモリMB)");
        }
        
        // dirinfo デモ
        win.font("MS Gothic", 12, 1);
        win.color(0, 128, 0).pos(50, 255);
        win.mes("dirinfo - ディレクトリ情報の取得:");
        win.font("MS Gothic", 12, 0);
        win.color(0, 0, 0).pos(50, 275);
        {
            win.mes("dir_cur() = \"" + dir_cur() + "\"");
            win.pos(50, 293);
            win.mes("dir_exe() = \"" + dir_exe() + "\"");
            win.pos(50, 311);
            win.mes("dir_win() = \"" + dir_win() + "\"");
            win.pos(50, 329);
            win.mes("dir_sys() = \"" + dir_sys() + "\"");
            win.pos(50, 347);
            win.mes("dir_desktop() = \"" + dir_desktop() + "\"");
            win.pos(50, 365);
            win.mes("dir_mydoc() = \"" + dir_mydoc() + "\"");
        }
        

        break;

    case ExtendedDemo::FileOps:
        win.color(0, 0, 0).pos(20, 85);
        win.mes("ファイル操作関数デモ: exist, dirlist, bload, bsave, exec, dialog");

        // アルファベットキーアクションメニュー
        win.font("MS Gothic", 12, 1);
        win.color(255, 128, 0).pos(400, 120);
        win.mes("【アクション実行キー】");
        win.font("MS Gothic", 10, 0);
        win.color(0, 0, 0).pos(400, 140);
        win.mes("M: dialog - メッセージ表示");
        win.pos(400, 155);
        win.mes("O: dialog - ファイル選択");
        win.pos(400, 170);
        win.mes("N: exec - メモ帳起動");
        win.pos(400, 185);
        win.mes("K: mkdir - ディレクトリ作成");
        win.pos(400, 200);
        win.mes("H: chdir - ディレクトリ変更");
        win.pos(400, 215);
        win.mes("V: bsave - テストファイル保存");
        win.pos(400, 230);
        win.mes("L: bload - テストファイル読込");
        win.pos(400, 245);
        win.mes("X: deletefile - ファイル削除");
        win.pos(400, 260);
        win.mes("Y: bcopy - ファイルコピー");
        win.pos(400, 275);
        win.mes("Q: dialog - カラー選択");
        
        // アクション実行結果表示
        if (!g_actionLog.empty()) {
            win.font("MS Gothic", 12, 1);
            win.color(255, 0, 0).pos(400, 300);
            win.mes("【実行結果】");
            win.font("MS Gothic", 10, 0);
            win.color(0, 128, 0).pos(400, 320);
            // UTF-8を安全に分割せずにそのまま表示
            win.mes(g_actionLog);
        }

        // exist デモ
        win.font("MS Gothic", 12, 1);
        win.color(0, 0, 128).pos(50, 120);
        win.mes("exist - ファイルサイズ取得:");
        win.font("MS Gothic", 12, 0);
        win.color(0, 0, 0).pos(50, 140);
        {
            int64_t size1 = exist("HspppSample.exe");
            win.mes("exist(\"HspppSample.exe\") = " + std::to_string(size1) + " bytes");
            win.pos(50, 158);
            int64_t size2 = exist("nonexistent_file_12345.txt");
            win.mes("exist(\"nonexistent_file_12345.txt\") = " + std::to_string(size2) + " (ファイルなし=-1)");
        }

        // dirlist デモ
        win.font("MS Gothic", 12, 1);
        win.color(0, 128, 0).pos(50, 190);
        win.mes("dirlist - ディレクトリ一覧取得 (*.exe):");
        win.font("MS Gothic", 12, 0);
        win.color(0, 0, 0).pos(50, 210);
        {
            std::vector<std::string> fileList = dirlist("*.exe", 1);  // ディレクトリを除く
            int fileCount = static_cast<int>(fileList.size());
            win.mes("ファイル数: " + std::to_string(fileCount));
            win.pos(50, 228);
            // 最初の数ファイルだけ表示
            int dispCount = (std::min)(fileCount, 4);
            for (int i = 0; i < dispCount; i++) {
                win.pos(50, 246 + i * 18);
                win.mes("  " + fileList[i]);
            }
            if (fileList.size() > 4) {
                win.pos(50, 246 + dispCount * 18);
                win.mes("  ... 他 " + std::to_string(fileList.size() - 4) + " ファイル");
            }
        }

        // chdir/mkdir/deletefile/bcopy デモ説明
        win.font("MS Gothic", 12, 1);
        win.color(128, 0, 128).pos(50, 340);
        win.mes("ファイル/ディレクトリ操作:");
        win.font("MS Gothic", 12, 0);
        win.color(0, 0, 0).pos(50, 360);
        win.mes("chdir, mkdir, deletefile, bcopy");

        // exec/dialog デモ説明
        win.font("MS Gothic", 12, 1);
        win.color(128, 64, 0).pos(50, 390);
        win.mes("exec/dialog - 外部プログラム/ダイアログ:");
        win.font("MS Gothic", 12, 0);
        win.color(0, 0, 0).pos(50, 410);
        win.mes("exec(cmd), dialog(msg/file/color)");
        break;
    
    case ExtendedDemo::InputMouse:
        win.color(0, 0, 0).pos(20, 85);
        win.mes("マウス入力関数デモ: mouse, mousex, mousey, mousew, ginfo");
        
        // リアルタイムマウス座標
        win.font("MS Gothic", 12, 1);
        win.color(0, 0, 128).pos(50, 120);
        win.mes("マウスカーソル座標 (リアルタイム):");
        win.font("MS Gothic", 12, 0);
        win.color(0, 0, 0).pos(50, 140);
        {
            win.mes("mousex() = " + str(mousex()) + ", mousey() = " + str(mousey()));
            win.pos(50, 158);
            win.mes("mousew() = " + str(mousew()) + " (ホイール移動量)");
            win.pos(50, 176);
            win.mes("ginfo(0) = " + str(ginfo(0)) + ", ginfo(1) = " + str(ginfo(1)) + " (スクリーン座標)");
        }
        
        // ginfo デモ
        win.font("MS Gothic", 12, 1);
        win.color(0, 128, 0).pos(50, 210);
        win.mes("ginfo - ウィンドウ情報取得:");
        win.font("MS Gothic", 12, 0);
        win.color(0, 0, 0).pos(50, 230);
        {
            win.mes("ginfo(2) = " + str(ginfo(2)) + " (アクティブウィンドウID)");
            win.pos(50, 248);
            win.mes("ginfo(3) = " + str(ginfo(3)) + " (操作先ウィンドウID)");
            win.pos(50, 266);
            win.mes("ginfo(10) = " + str(ginfo(10)) + ", ginfo(11) = " + str(ginfo(11)) + " (ウィンドウサイズ)");
            win.pos(50, 284);
            win.mes("ginfo(12) = " + str(ginfo(12)) + ", ginfo(13) = " + str(ginfo(13)) + " (クライアントサイズ)");
            win.pos(50, 302);
            win.mes("ginfo(20) = " + str(ginfo(20)) + ", ginfo(21) = " + str(ginfo(21)) + " (デスクトップサイズ)");
            win.pos(50, 320);
            win.mes("ginfo(22) = " + str(ginfo(22)) + ", ginfo(23) = " + str(ginfo(23)) + " (カレントポジション)");
        }
        
        // マウスカーソル追従表示
        win.font("MS Gothic", 12, 1);
        win.color(128, 0, 0).pos(50, 355);
        win.mes("マウスカーソル追従デモ:");
        {
            int mx = mousex();
            int my = mousey();
            // カーソル位置に円を描画
            win.color(255, 0, 0);
            win.circle(mx - 10, my - 10, mx + 10, my + 10, 0);
            win.circle(mx - 15, my - 15, mx + 15, my + 15, 0);
            win.color(0, 0, 255);
            win.pset(mx, my);
        }
        
        // 型変換/その他のユーティリティ
        win.font("MS Gothic", 12, 1);
        win.color(0, 128, 128).pos(350, 120);
        win.mes("型変換・ユーティリティ:");
        win.font("MS Gothic", 12, 0);
        win.color(0, 0, 0).pos(350, 140);
        {
            win.mes("toInt(3.14) = " + str(toInt(3.14)));
            win.pos(350, 158);
            win.mes("toDouble(42) = " + str(toDouble(42)));
            win.pos(350, 176);
            win.mes("strlen(\"Hello\") = " + str(hsppp::strlen("Hello")));
            win.pos(350, 194);
            win.mes("limitf(1.5, 0.0, 1.0) = " + str(limitf(1.5, 0.0, 1.0)));
        }
        
        // wait/stop 説明
        win.font("MS Gothic", 12, 1);
        win.color(128, 64, 0).pos(350, 230);
        win.mes("wait/stop 関数:");
        win.font("MS Gothic", 12, 0);
        win.color(0, 0, 0).pos(350, 250);
        win.mes("wait(100) = 1秒待機 (CPU負荷低)");
        win.pos(350, 268);
        win.mes("stop() = 割り込み待機で停止");
        win.pos(350, 286);
        win.mes("await(ms) = ミリ秒待機");
        break;
    
    case ExtendedDemo::Easing: {
        win.color(0, 0, 0).pos(20, 85);
        win.mes("イージング関数デモ: setease, getease, geteasef");
        
        // イージング曲線の一覧
        struct EaseInfo {
            int type;
            const char* name;
        };
        static constexpr EaseInfo easeTypes[] = {
            { ease_linear, "linear" },
            { ease_quad_in, "quad_in" },
            { ease_quad_out, "quad_out" },
            { ease_quad_inout, "quad_inout" },
            { ease_cubic_in, "cubic_in" },
            { ease_cubic_out, "cubic_out" },
            { ease_cubic_inout, "cubic_inout" },
            { ease_bounce_out, "bounce_out" },
        };
        constexpr size_t easeTypesCount = sizeof(easeTypes) / sizeof(easeTypes[0]);
        
        win.font("MS Gothic", 11, 0);
        
        // 各イージングタイプの曲線を描画
        int baseX = 50;
        int baseY = 110;
        int graphW = 120;
        int graphH = 80;
        int cols = 4;
        
        for (size_t i = 0; i < easeTypesCount; i++) {
            int col = static_cast<int>(i) % cols;
            int row = static_cast<int>(i) / cols;
            int gx = baseX + col * (graphW + 30);
            int gy = baseY + row * (graphH + 50);
            
            // グラフ背景
            win.color(240, 240, 240);
            win.boxf(gx, gy, gx + graphW, gy + graphH);
            
            // 枠線
            win.color(128, 128, 128);
            win.line(gx + graphW, gy, gx, gy);
            win.line(gx + graphW, gy + graphH);
            win.line(gx, gy + graphH);
            win.line(gx, gy);
            
            // ラベル
            win.color(0, 0, 0).pos(gx, gy + graphH + 5);
            win.mes(easeTypes[i].name);
            
            // イージング曲線を描画
            setease(0.0, 1.0, easeTypes[i].type);
            win.color(255, 0, 0);
            for (int x = 0; x <= graphW; x++) {
                double t = static_cast<double>(x) / graphW;
                double v = geteasef(t, 1.0);
                int py = gy + graphH - static_cast<int>(v * graphH);
                if (x == 0) {
                    win.pos(gx + x, py);
                } else {
                    win.line(gx + x, py);
                }
            }
        }
        
        // アニメーションデモ
        win.font("MS Gothic", 12, 1);
        win.color(0, 0, 128).pos(50, 330);
        win.mes("アニメーションデモ (自動更新):");
        
        // ボールの位置をイージングで計算
        static int animFrame = 0;
        animFrame++;
        if (animFrame > 100) animFrame = 0;
        
        // 異なるイージングタイプで同じ時間経過を表示
        int animY = 355;
        const int ballRadius = 8;
        const int startX = 100;
        const int endX = 500;
        
        // Linear
        setease(startX, endX, ease_linear);
        int x1 = getease(animFrame, 100);
        win.color(255, 0, 0);
        win.circle(x1 - ballRadius, animY - ballRadius, x1 + ballRadius, animY + ballRadius, 1);
        win.color(0, 0, 0).pos(50, animY - 5);
        win.font("MS Gothic", 10, 0);
        win.mes("linear");
        
        // Cubic InOut
        animY += 30;
        setease(startX, endX, ease_cubic_inout);
        int x2 = getease(animFrame, 100);
        win.color(0, 128, 0);
        win.circle(x2 - ballRadius, animY - ballRadius, x2 + ballRadius, animY + ballRadius, 1);
        win.color(0, 0, 0).pos(50, animY - 5);
        win.mes("cubic_inout");
        
        // Bounce Out
        animY += 30;
        setease(startX, endX, ease_bounce_out);
        int x3 = getease(animFrame, 100);
        win.color(0, 0, 255);
        win.circle(x3 - ballRadius, animY - ballRadius, x3 + ballRadius, animY + ballRadius, 1);
        win.color(0, 0, 0).pos(50, animY - 5);
        win.mes("bounce_out");
        
        // logmes デモ
        win.font("MS Gothic", 12, 1);
        win.color(0, 128, 128).pos(50, 455);
        win.mes("logmes - デバッグ出力 (Visual Studio Outputへ):");
        win.font("MS Gothic", 11, 0);
        win.color(0, 0, 0).pos(50, 475);
        win.mes("Lキー: logmes でメッセージ出力");
        break;
    }
    
    case ExtendedDemo::Sorting: {
        win.color(0, 0, 0).pos(20, 85);
        win.mes("ソート関数デモ: sortval, sortstr, sortnote, sortget");
        
        // 定数 SORT_INIT_* と共有変数 g_sort* を使用
        
        win.font("MS Gothic", 12, 1);
        
        // 整数配列ソート
        win.color(0, 0, 128).pos(50, 120);
        win.mes("sortval - 整数配列ソート:");
        win.font("MS Gothic", 11, 0);
        win.color(0, 0, 0).pos(50, 140);
        {
            std::string arrStr = "元データ: [";
            for (size_t i = 0; i < SORT_INIT_INT.size(); i++) {
                if (i > 0) arrStr += ", ";
                arrStr += str(SORT_INIT_INT[i]);
            }
            arrStr += "]";
            win.mes(arrStr);
        }
        win.pos(50, 160);
        {
            std::string arrStr = "ソート後: [";
            for (size_t i = 0; i < g_sortIntArr.size(); i++) {
                if (i > 0) arrStr += ", ";
                arrStr += str(g_sortIntArr[i]);
            }
            arrStr += "]";
            win.mes(arrStr);
        }
        
        // 文字列配列ソート
        win.font("MS Gothic", 12, 1);
        win.color(0, 128, 0).pos(50, 200);
        win.mes("sortstr - 文字列配列ソート:");
        win.font("MS Gothic", 11, 0);
        win.color(0, 0, 0).pos(50, 220);
        win.mes("元データ: [Banana, Apple, Cherry, Date, Elderberry]");
        win.pos(50, 240);
        {
            std::string arrStr = "ソート後: [";
            for (size_t i = 0; i < g_sortStrArr.size(); i++) {
                if (i > 0) arrStr += ", ";
                arrStr += g_sortStrArr[i];
            }
            arrStr += "]";
            win.mes(arrStr);
        }
        
        // メモリノートソート
        win.font("MS Gothic", 12, 1);
        win.color(128, 0, 0).pos(50, 280);
        win.mes("sortnote - メモリノート形式ソート:");
        win.font("MS Gothic", 11, 0);
        win.color(0, 0, 0).pos(50, 300);
        win.mes("元データ: Zebra\\nApple\\nMango\\nBanana\\nCherry");
        win.pos(50, 320);
        {
            std::string displayNote = g_sortNoteData;
            // 改行を\\nに置換して表示
            size_t pos = 0;
            while ((pos = displayNote.find('\n', pos)) != std::string::npos) {
                displayNote.replace(pos, 1, "\\n");
                pos += 2;
            }
            win.mes("ソート後: " + displayNote);
        }
        
        // sortget デモ
        win.font("MS Gothic", 12, 1);
        win.color(0, 128, 128).pos(50, 360);
        win.mes("sortget - ソート元インデックス取得:");
        win.font("MS Gothic", 11, 0);
        win.color(0, 0, 0).pos(50, 380);
        if (g_sortDone && !g_sortOrigIndices.empty()) {
            std::string idxStr = "sortget結果: [";
            for (size_t i = 0; i < g_sortOrigIndices.size(); i++) {
                if (i > 0) idxStr += ", ";
                idxStr += str(g_sortOrigIndices[i]);
            }
            idxStr += "]";
            win.mes(idxStr);
            win.pos(50, 400);
            win.mes("(現在のi番目の要素が、ソート前はどのインデックスにあったか)");
        } else {
            win.mes("Sキーを押すとソートを実行し、sortgetの結果を表示");
        }
        
        // 操作説明
        win.font("MS Gothic", 12, 1);
        win.color(64, 64, 64).pos(350, 120);
        win.mes("操作:");
        win.font("MS Gothic", 11, 0);
        win.color(0, 0, 0).pos(350, 140);
        win.mes("S: 昇順ソート実行");
        win.pos(350, 160);
        win.mes("D: 降順ソート実行");
        win.pos(350, 180);
        win.mes("R: データリセット");
        break;
    }
        
    default:
        break;
    }
}

// ═══════════════════════════════════════════════════════════════════
// 拡張デモのアクション処理
// ═══════════════════════════════════════════════════════════════════

void processExtendedAction(Screen& win) {
    // 修飾キーが押されている場合はアクション無効
    if (isModifierKeyPressed()) return;
    
    switch (static_cast<ExtendedDemo>(g_demoIndex)) {
    case ExtendedDemo::Math:
        if (getkey('R')) {
            randomize();
            await(200);
        }
        break;
        
    case ExtendedDemo::Grect:
        if (getkey(VK::LEFT)) { g_angle -= 5.0; await(50); }
        if (getkey(VK::RIGHT)) { g_angle += 5.0; await(50); }
        break;
        
    case ExtendedDemo::Gcopy:
        if (getkey('C') && !g_bufferCreated) {
            auto buf = buffer({.width = 200, .height = 200});
            buf.color(255, 128, 0).boxf();
            buf.color(0, 0, 200);
            buf.circle(20, 20, 180, 180, 1);
            buf.color(255, 255, 255).pos(50, 90);
            buf.mes("Buffer");
            
            win.select();
            pos(50, 320);
            gmode(0, 100, 100);
            gcopy(buf.id(), 0, 0, 200, 200);
            
            g_bufferCreated = true;
            await(200);
        }
        break;
        
    case ExtendedDemo::FileOps:
        // アルファベットキーでアクション
        if (getkey('M')) {
            dialog("これはdialog命令のテストです。\nメッセージボックスを表示しました。", 0, "HSPPP ダイアログテスト");
            g_actionLog = "dialog: メッセージ表示完了";
            await(200);
        }
        if (getkey('O')) {
            std::string selected = dialog("txt", 16, "テキストファイルを選択");
            g_actionLog = "dialog: " + (selected.empty() ? "キャンセル" : selected);
            await(200);
        }
        if (getkey('N')) {
            exec("notepad.exe", 16);  // mode=16: 非同期実行
            g_actionLog = "exec: notepad.exe 起動";
            await(200);
        }
        if (getkey('K')) {
            try {
                mkdir("test_hsppp_dir");
                g_actionLog = "mkdir: 作成成功";
            } catch (const std::exception&) {
                g_actionLog = "mkdir: エラー(既存?)";
            }
            await(200);
        }
        if (getkey('H')) {
            try {
                chdir("..");
                std::string newCur = dir_cur();
                g_actionLog = "chdir: " + newCur;
            } catch (const std::exception&) {
                g_actionLog = "chdir: エラー";
            }
            await(200);
        }
        if (getkey('V')) {
            try {
                std::vector<uint8_t> testData(256);
                for (int i = 0; i < 256; i++) testData[i] = static_cast<uint8_t>(i);
                bsave("test_hsppp.bin", testData);
                g_actionLog = "bsave: 保存完了(256bytes)";
            } catch (const std::exception&) {
                g_actionLog = "bsave: エラー";
            }
            await(200);
        }
        if (getkey('L')) {
            try {
                std::vector<uint8_t> loadData;
                bload("test_hsppp.bin", loadData);
                g_actionLog = "bload: 読込完了(" + str(static_cast<int>(loadData.size())) + "bytes)";
            } catch (const std::exception&) {
                g_actionLog = "bload: エラー(ファイルなし?)";
            }
            await(200);
        }
        if (getkey('X')) {
            try {
                deletefile("test_hsppp.bin");
                g_actionLog = "deletefile: 削除完了";
            } catch (const std::exception&) {
                g_actionLog = "deletefile: エラー";
            }
            await(200);
        }
        if (getkey('Y')) {
            try {
                std::vector<uint8_t> testData = {1, 2, 3, 4, 5};
                bsave("test_copy_src.bin", testData);
                bcopy("test_copy_src.bin", "test_copy_dst.bin");
                g_actionLog = "bcopy: コピー完了";
            } catch (const std::exception&) {
                g_actionLog = "bcopy: エラー";
            }
            await(200);
        }
        if (getkey('Q')) {
            if (dialog("", dialog_colorex, "")) {
                g_actionLog = "色選択: R=" + str(ginfo_r()) + " G=" + str(ginfo_g()) + " B=" + str(ginfo_b());
            } else {
                g_actionLog = "色選択: キャンセル";
            }
            await(200);
        }
        break;
    
    case ExtendedDemo::Easing:
        // logmes デモ
        if (getkey('L')) {
            logmes("logmes test: Hello from HSPPP!");
            logmes(42);
            logmes(3.14159);
            g_actionLog = "logmes: Output窓に出力しました";
            await(200);
        }
        break;
    
    case ExtendedDemo::Sorting: {
        // resetSortData() と定数 SORT_INIT_* を使用
        
        if (getkey('S')) {
            // 昇順ソート
            resetSortData();
            
            // sortvalでソート（g_sortIndicesが更新される）
            sortval(g_sortIntArr, 0);
            
            // sortval の結果を保存（sortnoteで上書きされる前に）
            g_sortOrigIndices.resize(g_sortIntArr.size());
            for (size_t i = 0; i < g_sortIntArr.size(); i++) {
                g_sortOrigIndices[i] = sortget(static_cast<int>(i));
            }
            
            sortstr(g_sortStrArr, 0);
            sortnote(g_sortNoteData, 0);
            
            g_sortDone = true;
            g_actionLog = "sortval/sortstr/sortnote: 昇順ソート完了";
            await(200);
        }
        if (getkey('D')) {
            // 降順ソート
            resetSortData();
            
            // sortvalでソート（g_sortIndicesが更新される）
            sortval(g_sortIntArr, 1);
            
            // sortval の結果を保存（sortnoteで上書きされる前に）
            g_sortOrigIndices.resize(g_sortIntArr.size());
            for (size_t i = 0; i < g_sortIntArr.size(); i++) {
                g_sortOrigIndices[i] = sortget(static_cast<int>(i));
            }
            
            sortstr(g_sortStrArr, 1);
            sortnote(g_sortNoteData, 1);
            
            g_sortDone = true;
            g_actionLog = "sortval/sortstr/sortnote: 降順ソート完了";
            await(200);
        }
        if (getkey('R')) {
            // リセット
            resetSortData();
            g_actionLog = "データをリセットしました";
            await(200);
        }
        break;
    }
    
    default:
        break;
    }
}
