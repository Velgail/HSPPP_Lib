# HspppLib バージョン管理ガイド

## バージョン番号の管理

HspppLib は[セマンティックバージョニング 2.0.0](https://semver.org/lang/ja/)を採用しています。

### バージョン番号の形式

```
MAJOR.MINOR.PATCH
例: 0.1.0, 1.2.3, 2.0.0-beta.1
```

### 各番号の意味

- **MAJOR（メジャー）**: 後方互換性のないAPI変更
  - 既存コードの修正が必要になる変更
  - 例: 関数シグネチャの変更、名前空間の変更

- **MINOR（マイナー）**: 後方互換性のある機能追加
  - 既存コードを壊さずに新機能を追加
  - 例: 新しい関数の追加、オプショナル引数の追加

- **PATCH（パッチ）**: 後方互換性のあるバグ修正
  - APIに影響しない内部的な修正
  - 例: バグ修正、パフォーマンス改善、ドキュメント修正

### バージョン0.x.x（初期開発フェーズ）

バージョン 0.x.x は開発段階であり、APIが不安定な可能性があります。
- 0.1.0: 初期リリース
- 0.2.0: 機能追加（破壊的変更の可能性あり）
- 1.0.0: 安定版リリース

## バージョン情報の更新手順

新しいバージョンをリリースする際は、以下の手順に従ってください：

### 1. バージョン番号を決定

変更内容に基づいて、MAJOR、MINOR、PATCHのどれを上げるか決定します。

### 2. version.hppを更新

[HspppLib/src/core/version.hpp](HspppLib/src/core/version.hpp) を編集：

```cpp
// バージョン番号の定義
constexpr int VERSION_MAJOR = 0;  // <- 必要に応じて変更
constexpr int VERSION_MINOR = 2;  // <- 必要に応じて変更
constexpr int VERSION_PATCH = 0;  // <- 必要に応じて変更

// バージョン文字列（"0.2.0" の形式）
constexpr const char* VERSION_STRING = "0.2.0";  // <- 上記と一致させる
```

### 3. README.mdを更新

[README.md](README.md) のバージョンバッジを更新：

```markdown
[![Version](https://img.shields.io/badge/version-0.2.0-blue.svg)](https://github.com/Velgail/HspppLib)
```

### 4. CHANGELOG.mdに変更内容を記録

（将来的に作成予定）

### 5. コミットとタグ

```powershell
# 変更をコミット
git add HspppLib/src/core/version.hpp README.md
git commit -m "chore: bump version to 0.2.0"

# バージョンタグを作成
git tag -a v0.2.0 -m "Release version 0.2.0"

# リモートにプッシュ
git push origin main
git push origin v0.2.0
```

## バージョン情報の使用方法

### C++コードから取得

```cpp
import hsppp;

int hspMain() {
    // バージョン文字列を取得
    const char* ver = hsppp::version();
    hsppp::mes(std::format("HspppLib version: {}", ver));
    
    // 詳細なバージョン情報を取得
    auto info = hsppp::get_version();
    hsppp::mes(std::format("Version: {}.{}.{}", 
        info.major, info.minor, info.patch));
    hsppp::mes(std::format("Build: {}", info.build_type));
    hsppp::mes(std::format("Platform: {}", info.platform));
    hsppp::mes(std::format("C++ Version: {}", info.cxx_version));
    
    return 0;
}
```

### コンパイル時の確認

ビルド時にコンソール出力でバージョン情報が表示されます：

```
===============================================
HspppLib - Hot Soup Processor Plus Plus
Version: 0.1.0
Build Type: Debug
Platform: Windows x64
C++ Standard: C++23
===============================================
```

### バージョン比較（例）

```cpp
import hsppp;

// 最低限必要なバージョンをチェック
constexpr int REQUIRED_VERSION = (0 << 16) | (1 << 8) | 0;  // 0.1.0

static_assert(hsppp::VERSION_NUMBER >= REQUIRED_VERSION, 
    "HspppLib 0.1.0 or later required");
```

## ベストプラクティス

1. **破壊的変更は慎重に**: MAJOR バージョンアップは慎重に計画する
2. **変更ログを記録**: 各リリースで何が変わったか明確にする
3. **テストを実施**: バージョンアップ前に必ずテストを実行
4. **ドキュメント更新**: API変更時は必ずドキュメントも更新
5. **タグを付ける**: Gitタグでバージョンを管理し、リリースを追跡可能に

## リリースチェックリスト

- [ ] version.hpp を更新
- [ ] README.md のバージョンバッジを更新
- [ ] CHANGELOG.md に変更内容を記録（将来）
- [ ] すべてのテストがパス
- [ ] ビルドが成功（Debug/Release両方）
- [ ] ドキュメントが最新
- [ ] Git タグを作成
- [ ] リモートにプッシュ
