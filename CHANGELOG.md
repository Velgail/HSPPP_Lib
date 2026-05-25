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
- 高精度タイミング機能
  - `await()` の高精度化：QueryPerformanceCounter を使用（マイクロ秒単位）
  - `vwait()` の新規追加：VSync 同期待機関数
  - `ginfo_fps` 定数と `ginfo_fps()` 関数の実装：モニター最大リフレッシュレート取得
- 新 State Machine（`StateGraph<T>` / `StateMachine<T>` alias）
  - `run()` は **dispatch only**（内部で `await()` を呼ばない、ユーザーが `on_update` 内でフレーム制御を書く責務）
  - `step()` を公開（`tick()` の推奨 alias、サブ SM 用）
  - `on_update` 契約再定義: 「名前付き repeat-loop の 1 iteration」（`while(!is_transitioning())` の手書き強制を撤廃）
  - `state_elapsed_ms()` を追加（ms 統一）
  - `pause_timer()` / `resume_timer()` の意味論を「次の遷移を跨いでタイマー状態を保持する明示的意思表示」として確立
  - `perform_transition` 内の自動 `cancel_timer()` は paused タイマーを skip（明示 `cancel_timer()` は意思表示を上書き、二層契約）
- 変数パッケージ
  - `StateScope<TState>` / `state_vars` ショートカット — ステート別変数の bind / get / release（冪等）
  - `Repository<T>` / `GameServices` / `services()` — シングルトン型永続データ管理
  - `register_repository<T, Tag>()` / `reset_all()` — タグ別一括リセット
- セーブデータ
  - `Serializable<L>` concept（`serialize` / `deserialize` / `type_tag` 要件）
  - `SaveWriter` / `SaveReader` — マジック + version + block 群のバイナリ形式
  - `StateScope::snapshot()` / `restore()` — Serializable 変数の一括 snapshot / 部分復元
- 新規ドキュメント
  - `docs/api/statemachine.md`（新 API へ全面改訂）
  - `docs/api/state-vars-savedata.md`（NEW）
  - `docs/api/repository.md`（NEW）
  - `docs/guides/data-sharing.md`（NEW）
  - `docs/PROPOSAL-modern-hsppp.md`（設計提言ドラフト、TICKET-000 由来）
- サンプル統合: `HspppStateSample/StateSampleMain.cpp` を StateGraph + StateScope + state_vars + SaveWriter/Reader + bsave/bload で 1 本に統合（TICKET-005、案③ `register_repository` 呼出除去）
- テスト: `HspppTest/StateVarsRuntimeTest.cpp` 追加（TICKET-006 案④ ランタイム検証拡張）

### Changed
- `run()` のシグネチャを `void run(int target_ms = 16)` から **`void run()`** に変更（破壊的変更、design-TICKET-008 §7.1）
- `tick()` を撤回せず維持し、`step()` を等価な推奨 alias として追加（design-TICKET-008 §7.1）
- HspppStateSample を新 API（StateGraph + StateScope）に全面移行（旧 `NewStateSampleMain.cpp` / `TestNewFeatures.cpp` は除去）

### Deprecated
- `state_frame_count()` — `state_elapsed_ms()` を使用してください（ms 統一）
- `defer_jump()` — `jump()` を使用してください（dispatcher が return 後に遷移処理するため意味論的差なし）

### Removed
- `attach_child()` / `detach_child()` — サブ SM は親 `on_update` 内で明示的に `child.step()` を呼ぶ規約へ変更（design-TICKET-008 §11.1）
- `run()` の `target_ms` 引数および `run()` 内部の `await(target_ms)` 呼出（ユーザーが `on_update` 内でフレーム制御を書く責務に統一）

### Fixed
- TR-6: `Pause → Game` 復帰後の GameOver タイマー永久失効を修正（`perform_transition` 内の自動 `cancel_timer()` が paused タイマーを破棄していた問題、design-TICKET-010）
- TR-8: HspppTest の `ApiCompileTest` 副作用 API リソース供給漏れ解消（関数ポインタ ODR-use 化、案 α、TICKET-009）
- `register_repository` の C2280（`RepositoryRegistration` のデフォルトコンストラクタ削除との衝突）相当の経路を、サンプル側から呼出除去することで実用上解消（TICKET-005 案③ / build-config §4.1 解消相当、HEAD `3878e17` 以降で再現なし）

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
