// HspppSample/DemoDrawExtended.cpp
// ═══════════════════════════════════════════════════════════════════
// HSPPP デモアプリケーション - 拡張デモ描画
// ═══════════════════════════════════════════════════════════════════

#include "DemoState.h"
import hsppp;
import <format>;
using namespace hsppp;

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
        if (g_srcBufferId == -9999) {
            (void)buffer(100, 100, 100);  // 固定ID 100でバッファ作成
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
        if (g_srcBufferId == -9999) {
            (void)buffer(100, 100, 100);
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
        if (g_srcBufferId == -9999) {
            (void)buffer(100, 100, 100);
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
            int count = strrep(text, "AAA", "XXX");
            win.pos(350, 222);
            win.mes("strrep(text, \"AAA\", \"XXX\") = " + str(count) + "回");
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
            int len1 = getstr(dest, buf, 0, ',');
            win.mes("getstr(dest, \"ABC,DEF,GHI\", 0, ',') = \"" + dest + "\" (len=" + str(len1) + ")");
            win.pos(50, 297);
            int len2 = getstr(dest, buf, len1, ',');
            win.mes("getstr(dest, buf, " + str(len1) + ", ',') = \"" + dest + "\" (len=" + str(len2) + ")");
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
        
        // memcpy/memset/memexpand デモ（新規追加）
        win.font("MS Gothic", 12, 1);
        win.color(128, 64, 0).pos(50, 330);
        win.mes("memcpy/memset/memexpand - メモリ操作:");
        win.font("MS Gothic", 12, 0);
        win.color(0, 0, 0).pos(50, 347);
        {
            std::string src = "ABCDEFGH";
            std::string dest(8, '.');
            hsppp::memcpy(dest, src, 4);  // 先頭4バイトをコピー
            win.mes("memcpy: src=\"ABCDEFGH\", dest=\"........\"");
            win.pos(50, 362);
            win.mes("  memcpy(dest, src, 4) -> dest=\"" + dest + "\"");
            
            win.pos(50, 380);
            std::string setbuf(8, 'X');
            hsppp::memset(setbuf, 'A', 4);
            win.mes("memset: buf=\"XXXXXXXX\" -> memset(buf, 'A', 4)");
            win.pos(50, 395);
            win.mes("  結果: \"" + setbuf + "\"");
            
            win.pos(50, 413);
            std::string expbuf(16, 'Z');
            size_t oldSize = expbuf.size();
            hsppp::memexpand(expbuf, 64);
            win.mes("memexpand: 元size=" + str(static_cast<int>(oldSize)) + " -> memexpand(expbuf, 64) -> size=" + str(static_cast<int>(expbuf.size())));
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
        
        // peek/poke デモ
        win.font("MS Gothic", 12, 1);
        win.color(128, 0, 0).pos(50, 395);
        win.mes("peek/poke - メモリバッファ操作:");
        win.font("MS Gothic", 12, 0);
        win.color(0, 0, 0).pos(50, 415);
        {
            std::vector<uint8_t> buf(16, 0);
            poke(buf, 0, 0x41);      // 'A'
            wpoke(buf, 2, 0x1234);   // 2バイト値
            lpoke(buf, 4, 0xDEADBEEF); // 4バイト値
            
            win.mes("poke(buf, 0, 0x41) -> peek(buf, 0) = 0x" + strf("%02X", peek(buf, 0)));
            win.pos(50, 433);
            win.mes("wpoke(buf, 2, 0x1234) -> wpeek(buf, 2) = 0x" + strf("%04X", wpeek(buf, 2)));
            win.pos(50, 451);
            win.mes("lpoke(buf, 4, 0xDEADBEEF) -> lpeek(buf, 4) = 0x" + strf("%08X", lpeek(buf, 4)));
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
            int size1 = exist("HspppSample.exe");
            win.mes("exist(\"HspppSample.exe\") = " + std::to_string(size1) + " bytes");
            win.pos(50, 158);
            int size2 = exist("nonexistent_file_12345.txt");
            win.mes("exist(\"nonexistent_file_12345.txt\") = " + std::to_string(size2) + " (ファイルなし=-1)");
        }

        // dirlist デモ
        win.font("MS Gothic", 12, 1);
        win.color(0, 128, 0).pos(50, 190);
        win.mes("dirlist - ディレクトリ一覧取得 (*.exe):");
        win.font("MS Gothic", 12, 0);
        win.color(0, 0, 0).pos(50, 210);
        {
            std::string files = dirlist("*.exe", 1);  // ディレクトリを除く
            int fileCount = stat();
            win.mes("ファイル数: " + std::to_string(fileCount));
            win.pos(50, 228);
            // 最初の数ファイルだけ表示
            auto fileList = split(files, "\n");
            int dispCount = (std::min)(static_cast<int>(fileList.size()), 4);
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
            dialog("txt", 16, "テキストファイルを選択");
            std::string selected = refstr();
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
            int result = dialog("", dialog_colorex, "");
            if (result == 1) {
                g_actionLog = "色選択: R=" + str(ginfo_r()) + " G=" + str(ginfo_g()) + " B=" + str(ginfo_b());
            } else {
                g_actionLog = "色選択: キャンセル";
            }
            await(200);
        }
        break;
        
    default:
        break;
    }
}
