# Changelog

All notable changes to HspppLib will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- バージョン管理システムの実装
  - `version.hpp` によるバージョン番号管理
  - `hsppp::get_version()` / `hsppp::version()` 関数
  - ビルド時のバージョン情報表示

### Changed

### Deprecated

### Removed

### Fixed

### Security

## [0.1.0] - 2026-01-02

### Added
- 初期リリース
- HSP互換API（screen, color, boxf, mes など）
- Direct2D描画エンジン
- モジュールシステム（C++23 modules）
- デモプログラム集
- 基本的なドキュメント

---

## リリースノートの書き方

各変更は以下のカテゴリに分類してください：

- **Added**: 新機能
- **Changed**: 既存機能の変更
- **Deprecated**: 近い将来削除される機能
- **Removed**: 削除された機能
- **Fixed**: バグ修正
- **Security**: セキュリティ関連の修正

### 記載例

```markdown
## [0.2.0] - 2026-01-15

### Added
- `gcopy` 関数による画像コピー機能
- `gfilter` によるフィルタ効果

### Changed
- `boxf` のパフォーマンスを改善
- エラーメッセージをより詳細に

### Fixed
- `color` 関数のアルファ値が正しく適用されない問題を修正
- メモリリーク修正（Surface クラス）
```

### バージョン番号の選び方

- **MAJOR (x.0.0)**: 後方互換性のないAPI変更
  - 関数シグネチャの変更
  - 名前空間の変更
  - 必須パラメータの追加

- **MINOR (0.x.0)**: 後方互換性のある機能追加
  - 新しい関数の追加
  - オプショナル引数の追加
  - 新しいモジュールの追加

- **PATCH (0.0.x)**: 後方互換性のあるバグ修正
  - バグ修正
  - パフォーマンス改善（APIに影響なし）
  - ドキュメント修正
