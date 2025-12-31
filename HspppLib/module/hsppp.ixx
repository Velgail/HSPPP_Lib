// HspppLib/module/hsppp.ixx
// メインモジュールインターフェース: 各パーティションを統合してエクスポート
//
// 使用例:
//   import hsppp;
//   hsppp::screen(0, 800, 600);
//   hsppp::color(255, 0, 0);
//   hsppp::mes("Hello, HSPPP!");

export module hsppp;

// 各パーティションを再エクスポート
export import :types;
export import :screen;
export import :drawing;
export import :input;
export import :math;
export import :string;
export import :file;
export import :interrupt;
