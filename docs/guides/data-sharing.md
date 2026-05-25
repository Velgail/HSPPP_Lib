---
layout: default
title: Data Sharing Guide
---

# 繝・・繧ｿ蜈ｱ譛峨ぎ繧､繝・

StateMachine 縺ｧ繝・・繧ｿ繧貞・譛峨☆繧・縺､縺ｮ繝代ち繝ｼ繝ｳ縺ｨ菴ｿ縺・・縺第婿豕輔・

---

## 2縺､縺ｮ繝代ち繝ｼ繝ｳ

### 1. 繧ｷ繝ｳ繝励Ν繝代ち繝ｼ繝ｳ・医Ο繝ｼ繧ｫ繝ｫ螟画焚・・

`on_update` 蜀・〒繝ｫ繝ｼ繝励ｒ譖ｸ縺丞ｴ蜷医∵勸騾壹・繝ｭ繝ｼ繧ｫ繝ｫ螟画焚縺ｧ蜊∝・縺ｧ縺吶・

```cpp
enum class Scene { Title, Game };

void hspMain() {
    StateGraph<Scene> sm;
    
    sm.state(Scene::Game)
      .on_update([](auto& sm) {
          // 笨・繝ｭ繝ｼ繧ｫ繝ｫ螟画焚縺ｧOK
          int score = 0;
          int lives = 3;
          
          while (!sm.is_transitioning()) {
              score += 10;
              
              if (stick(256)) {
                  lives--;
                  if (lives == 0) {
                      sm.jump(Scene::Title);
                  }
              }
              
              mes(strf("Score: %d, Lives: %d", score, lives));
              await(16);
          }
      });
    
    sm.start(Scene::Title);
}
```

**繝｡繝ｪ繝・ヨ**:
- 笨・繧ｷ繝ｳ繝励Ν
- 笨・霑ｽ蜉繧ｳ繝ｼ繝我ｸ崎ｦ・
- 笨・繧ｹ繧ｳ繝ｼ繝励′譏守｢ｺ

**繝・Γ繝ｪ繝・ヨ**:
- 笶・`on_enter` 縺ｨ `on_exit` 縺ｧ縺ｯ菴ｿ縺医↑縺・

---

### 2. 讒矩蛹悶ヱ繧ｿ繝ｼ繝ｳ・・tate<T>()・・

`on_enter`, `on_update`, `on_exit` 縺ｧ繝・・繧ｿ繧貞・譛峨＠縺溘＞蝣ｴ蜷医↓菴ｿ縺・∪縺吶・

```cpp
struct GameSceneData {
    int score = 0;
    int lives = 3;
    int stage = 1;
};

void hspMain() {
    StateGraph<Scene> sm;
    
    sm.state<GameSceneData>(Scene::Game)
      .on_enter([](auto& sm, GameSceneData& data) {
          // 笨・蛻晄悄蛹・
          data.score = 0;
          data.lives = 3;
          data.stage = 1;
      })
      .on_update([](auto& sm, GameSceneData& data) {
          while (!sm.is_transitioning()) {
              data.score += 10;
              
              if (stick(256)) {
                  data.lives--;
                  if (data.lives == 0) {
                      sm.jump(Scene::Title);
                  }
              }
              
              mes(strf("Score: %d, Lives: %d", data.score, data.lives));
              await(16);
          }
      })
      .on_exit([](auto& sm, GameSceneData& data) {
          // 笨・邨ゆｺ・・逅・ｼ医ワ繧､繧ｹ繧ｳ繧｢菫晏ｭ倥↑縺ｩ・・
          auto& profile = services().data<PlayerProfile>();
          if (data.score > profile.high_score) {
              profile.high_score = data.score;
          }
      });
    
    sm.start(Scene::Title);
}
```

**繝｡繝ｪ繝・ヨ**:
- 笨・`on_enter` / `on_exit` 縺ｧ繧ゆｽｿ縺医ｋ
- 笨・繝・・繧ｿ縺後∪縺ｨ縺ｾ縺｣縺ｦ縺・ｋ
- 笨・閾ｪ蜍慕噪縺ｫ繧ｯ繝ｪ繝ｼ繝ｳ繧｢繝・・縺輔ｌ繧・

**繝・Γ繝ｪ繝・ヨ**:
- 笶・讒矩菴薙・螳夂ｾｩ縺悟ｿ・ｦ・

---

## 豌ｸ邯壹ョ繝ｼ繧ｿ: Repository<T>

繧ｷ繝ｼ繝ｳ繧偵∪縺溘＞縺ｧ菫晄戟縺励◆縺・ョ繝ｼ繧ｿ縺ｫ縺ｯ `Repository<T>` 繧剃ｽｿ縺・∪縺吶・

```cpp
struct PlayerProfile {
    std::string name = "Player";
    int high_score = 0;
};

void hspMain() {
    StateGraph<Scene> sm;
    
    sm.state(Scene::Game)
      .on_update([](auto& sm) {
          // 笨・縺ｩ縺ｮ繧ｷ繝ｼ繝ｳ縺九ｉ縺ｧ繧ゅい繧ｯ繧ｻ繧ｹ蜿ｯ閭ｽ
          auto& profile = services().data<PlayerProfile>();
          
          int score = 0;
          while (!sm.is_transitioning()) {
              score += 10;
              mes(strf("Score: %d, High: %d", score, profile.high_score));
              await(16);
          }
          
          // 繝上う繧ｹ繧ｳ繧｢譖ｴ譁ｰ
          if (score > profile.high_score) {
              profile.high_score = score;
          }
      });
    
    sm.state(Scene::Result)
      .on_update([](auto& sm) {
          // 笨・蛻･縺ｮ繧ｷ繝ｼ繝ｳ縺ｧ繧ょ酔縺倥ョ繝ｼ繧ｿ縺ｫ繧｢繧ｯ繧ｻ繧ｹ
          auto& profile = services().data<PlayerProfile>();
          
          mes(strf("High Score: %d", profile.high_score));
          mes("Press SPACE");
          
          while (!sm.is_transitioning()) {
              if (stick(16)) sm.jump(Scene::Title);
              await(16);
          }
      });
    
    sm.start(Scene::Title);
}
```

---

## 繧ｰ繝ｭ繝ｼ繝舌Ν螟画焚縺ｨ縺ｮ驕輔＞

### Repository<T> vs 繧ｰ繝ｭ繝ｼ繝舌Ν螟画焚

`Repository<T>` 縺ｯ縲悟梛螳牙・縺ｪ繧ｰ繝ｭ繝ｼ繝舌Ν螟画焚縲阪〒縺吶′縲√＞縺上▽縺九・蛻ｩ轤ｹ縺後≠繧翫∪縺呻ｼ・

| 迚ｹ蠕ｴ | 繧ｰ繝ｭ繝ｼ繝舌Ν螟画焚 | Repository<T> |
|------|--------------|--------------|
| **蝙句ｮ牙・諤ｧ** | 笶・蜷榊燕縺ｮ陦晉ｪ√′襍ｷ縺阪ｋ | 笨・蝙九＃縺ｨ縺ｫ迢ｬ遶・|
| **蜷榊燕遨ｺ髢・* | 笶・繧ｰ繝ｭ繝ｼ繝舌Ν蜷榊燕遨ｺ髢薙ｒ豎壽沒 | 笨・蝙句錐縺ｧ邂｡逅・|
| **繝ｪ繧ｻ繝・ヨ** | 笶・謇句虚縺ｧ蛻晄悄蛹悶′蠢・ｦ・| 笨・reset_all() 縺ｧ荳諡ｬ蛻晄悄蛹・|
| **蜿ｯ隕匁ｧ** | 笶・縺ｩ縺薙°繧峨〒繧よ囓鮟咏噪縺ｫ繧｢繧ｯ繧ｻ繧ｹ | 笨・services().data<T>() 縺ｧ譏守､ｺ逧・|
| **繝・せ繝・* | 笶・迥ｶ諷九・繧ｯ繝ｪ繧｢縺碁擇蛟・| 笨・register/reset 縺ｧ繧ｯ繝ｪ繧｢蜿ｯ閭ｽ |

**萓具ｼ壹げ繝ｭ繝ｼ繝舌Ν螟画焚縺ｮ蝠城｡・*

```cpp
// 笶・繧ｰ繝ｭ繝ｼ繝舌Ν螟画焚・壼錐蜑阪・陦晉ｪ√′襍ｷ縺阪ｋ
int score = 0;          // 繧ｲ繝ｼ繝縺ｮ繧ｹ繧ｳ繧｢・・
int score_multiplier;   // 縺薙ｌ繧・score・・
int high_score;         // 莨ｼ縺溷錐蜑阪′蠅励∴繧・..

void game_scene() {
    score += 10;  // 縺ｩ縺ｮ繧ｹ繧ｳ繧｢縺九ｏ縺九ｊ縺ｫ縺上＞
}
```

**Repository<T> 縺ｮ隗｣豎ｺ遲・*

```cpp
// 笨・Repository: 蝙九〒繧ｰ繝ｫ繝ｼ繝怜喧
struct GameState {
    int score = 0;
    int multiplier = 1;
};

struct PlayerProfile {
    int high_score = 0;
};

void game_scene() {
    auto& game = services().data<GameState>();
    game.score += 10 * game.multiplier;  // 譏守｢ｺ
    
    auto& profile = services().data<PlayerProfile>();
    if (game.score > profile.high_score) {
        profile.high_score = game.score;
    }
}
```

### 豕ｨ諢冗せ・壹◎繧後〒繧ゅげ繝ｭ繝ｼ繝舌Ν繧ｹ繝・・繝・

**Repository<T> 縺ｯ荳・・縺ｧ縺ｯ縺ゅｊ縺ｾ縺帙ｓ**・・

1. **邨仙ｱ繧ｰ繝ｭ繝ｼ繝舌Ν**: 蜀・Κ逧・↓縺ｯ繧ｷ繝ｳ繧ｰ繝ｫ繝医Φ縺ｪ縺ｮ縺ｧ縲√げ繝ｭ繝ｼ繝舌Ν螟画焚縺ｨ蜷後§蝠城｡後ｒ謚ｱ縺医ｋ
2. **繝・せ繧ｿ繝薙Μ繝・ぅ**: 萓晏ｭ俶ｧ豕ｨ蜈･ (DI) 繧剃ｽｿ縺｣縺溘⊇縺・′繝・せ繝医＠繧・☆縺・
3. **繧ｹ繝ｬ繝・ラ繧ｻ繝ｼ繝輔〒縺ｯ縺ｪ縺・*: 繝槭Ν繝√せ繝ｬ繝・ラ縺ｧ縺ｯ菴ｿ逕ｨ荳榊庄
4. **證鈴ｻ咏噪縺ｪ萓晏ｭ倬未菫・*: 縺ｩ縺ｮ髢｢謨ｰ縺後←縺ｮ繝・・繧ｿ繧剃ｽｿ縺・°縲√さ繝ｼ繝峨°繧芽ｦ九∴縺ｫ縺上＞

**謗ｨ螂ｨ縺輔ｌ繧倶ｽｿ縺・婿**・・

```cpp
// 笨・繧ｲ繝ｼ繝縺ｮ繝医ャ繝励Ξ繝吶Ν縺ｧ菴ｿ縺・ｼ・SP縺ｮ繧ｰ繝ｭ繝ｼ繝舌Ν螟画焚縺ｮ莉｣繧上ｊ・・
void hspMain() {
    auto& settings = services().data<GameSettings>();
    settings.volume = 0.8f;
    
    StateGraph<Scene> sm;
    // ...
}

// 笞・・豺ｱ縺・未謨ｰ縺九ｉ菴ｿ縺・・縺ｯ驕ｿ縺代ｋ・井ｾ晏ｭ倬未菫ゅ′隕九∴縺ｫ縺上￥縺ｪ繧具ｼ・
void deep_function() {
    auto& settings = services().data<GameSettings>();  // 縺薙％縺ｧ遯∫┯蜃ｺ縺ｦ縺上ｋ縺ｨ豺ｷ荵ｱ
    // ...
}
```

**縺・▽繧ｰ繝ｭ繝ｼ繝舌Ν螟画焚繧剃ｽｿ縺・∋縺阪°**・・

| 繧ｱ繝ｼ繧ｹ | 繧ｰ繝ｭ繝ｼ繝舌Ν螟画焚 | Repository<T> |
|--------|--------------|--------------|
| **HSP縺九ｉ遘ｻ讀・* | 笶・| 笨・Repository 縺ｧ鄂ｮ縺肴鋤縺・|
| **險ｭ螳壹ョ繝ｼ繧ｿ** | 笶・| 笨・Repository 謗ｨ螂ｨ |
| **繧ｻ繝ｼ繝悶ョ繝ｼ繧ｿ** | 笶・| 笨・Repository 謗ｨ螂ｨ |
| **荳譎ら噪縺ｪ繝輔Λ繧ｰ** | 笨・繝ｭ繝ｼ繧ｫ繝ｫ螟画焚縺ｧ | 笶・荳崎ｦ・|
| **螟ｧ隕乗ｨ｡繝励Ο繧ｸ繧ｧ繧ｯ繝・* | 笶・| 笞・・DI 繧呈､懆ｨ・|

---

## 菴ｿ縺・・縺代メ繝｣繝ｼ繝・

```
繝・・繧ｿ繧貞・譛峨＠縺溘＞・・
笏・
笏懌楳 繧ｷ繝ｼ繝ｳ繧偵∪縺溘＄・・
笏・ 笏披楳 YES 竊・Repository<T>
笏・
笏披楳 蜷後§繧ｷ繝ｼ繝ｳ蜀・□縺托ｼ・
   笏・
   笏懌楳 on_enter / on_exit 縺ｧ菴ｿ縺・ｼ・
   笏・ 笏披楳 YES 竊・state<T>()
   笏・
   笏披楳 on_update 蜀・□縺托ｼ・
      笏披楳 YES 竊・繝ｭ繝ｼ繧ｫ繝ｫ螟画焚
```

---

## 繝代ち繝ｼ繝ｳ蛻･縺ｮ螳滉ｾ・

### 萓・: 繧ｿ繧､繝医Ν逕ｻ髱｢・医す繝ｳ繝励Ν・・

```cpp
sm.state(Scene::Title)
  .on_update([](auto& sm) {
      int selected = 0;  // 笨・繝ｭ繝ｼ繧ｫ繝ｫ螟画焚縺ｧ蜊∝・
      
      while (!sm.is_transitioning()) {
          mes("1. Start Game");
          mes("2. Options");
          
          if (stick(128)) selected--;
          if (stick(256)) selected++;
          selected = (selected + 2) % 2;
          
          if (stick(16)) {
              sm.jump(selected == 0 ? Scene::Game : Scene::Options);
          }
          
          await(16);
      }
  });
```

### 萓・: 繧ｲ繝ｼ繝繧ｷ繝ｼ繝ｳ・域ｧ矩蛹厄ｼ・

```cpp
struct GameSceneData {
    int score = 0;
    int stage = 1;
    bool boss_defeated = false;
};

sm.state<GameSceneData>(Scene::Game)
  .on_enter([](auto& sm, GameSceneData& data) {
      // 繧ｹ繝・・繧ｸ髢句ｧ区凾縺ｮ蛻晄悄蛹・
      data.score = 0;
      data.boss_defeated = false;
  })
  .on_update([](auto& sm, GameSceneData& data) {
      while (!sm.is_transitioning()) {
          // 繧ｲ繝ｼ繝繝ｭ繧ｸ繝・け
          data.score += 10;
          
          // 繝懊せ謦・ｴ蛻､螳・
          if (/* boss HP == 0 */) {
              data.boss_defeated = true;
              data.stage++;
          }
          
          await(16);
      }
  })
  .on_exit([](auto& sm, GameSceneData& data) {
      // 繧ｹ繧ｳ繧｢繧剃ｿ晏ｭ・
      auto& profile = services().data<PlayerProfile>();
      if (data.score > profile.high_score) {
          profile.high_score = data.score;
      }
  });
```

### 萓・: 繝励Ξ繧､繝､繝ｼ繝励Ο繝輔ぃ繧､繝ｫ・域ｰｸ邯夲ｼ・

```cpp
struct PlayerProfile {
    std::string name = "Player";
    int level = 1;
    int exp = 0;
    int gold = 100;
};

void hspMain() {
    // 笨・逋ｻ骭ｲ縺励※繝ｪ繧ｻ繝・ヨ蜿ｯ閭ｽ縺ｫ縺吶ｋ
    services().register_repository<PlayerProfile>();
    
    StateGraph<Scene> sm;
    
    sm.state(Scene::Game)
      .on_update([](auto& sm) {
          auto& profile = services().data<PlayerProfile>();
          
          while (!sm.is_transitioning()) {
              profile.exp += 10;
              if (profile.exp >= 100) {
                  profile.level++;
                  profile.exp = 0;
              }
              await(16);
          }
      });
    
    sm.state(Scene::Shop)
      .on_update([](auto& sm) {
          auto& profile = services().data<PlayerProfile>();
          
          // 笨・蜷後§繝・・繧ｿ縺ｫ繧｢繧ｯ繧ｻ繧ｹ
          mes(strf("Gold: %d", profile.gold));
          await(16);
      });
    
    sm.start(Scene::Title);
}
```

---

## 繧ｵ繝悶せ繝・・繝医・繧ｷ繝ｳ: tick()

繝｡繧､繝ｳ繧ｹ繝・・繝医・繧ｷ繝ｳ蜀・〒繧ｵ繝悶せ繝・・繝医・繧ｷ繝ｳ繧剃ｽｿ縺・ｴ蜷医～tick()` 繧剃ｽｿ縺・∪縺吶・

```cpp
enum class BattlePhase { Start, PlayerTurn, EnemyTurn, End };

struct BattleData {
    int turn = 0;
    int player_hp = 100;
    int enemy_hp = 50;
};

sm.state(Scene::Battle)
  .on_update([](auto& sm) {
      // 笨・繧ｵ繝悶せ繝・・繝医・繧ｷ繝ｳ繧剃ｽ懈・
      StateGraph<BattlePhase> battle;
      
      battle.state<BattleData>(BattlePhase::Start)
        .on_enter([](auto& sm, BattleData& data) {
            data.turn = 0;
            data.player_hp = 100;
            data.enemy_hp = 50;
        })
        .on_update([](auto& sm, BattleData& data) {
            int frame = 0;
            while (!sm.is_transitioning()) {
                mes("Battle Start!");
                if (++frame >= 60) sm.jump(BattlePhase::PlayerTurn);
                await(16);
            }
        });
      
      battle.state<BattleData>(BattlePhase::PlayerTurn)
        .on_update([](auto& sm, BattleData& data) {
            while (!sm.is_transitioning()) {
                mes(strf("Turn %d: Your Turn", data.turn));
                if (stick(16)) {  // SPACE
                    data.enemy_hp -= 20;
                    sm.jump(BattlePhase::EnemyTurn);
                }
                await(16);
            }
        });
      
      battle.state<BattleData>(BattlePhase::EnemyTurn)
        .on_update([](auto& sm, BattleData& data) {
            int frame = 0;
            while (!sm.is_transitioning()) {
                mes("Enemy Turn");
                if (++frame >= 60) {
                    data.player_hp -= 10;
                    data.turn++;
                    sm.jump(BattlePhase::PlayerTurn);
                }
                await(16);
            }
        });
      
      battle.start(BattlePhase::Start);
      
      // 笨・tick() 縺ｧ1繝輔Ξ繝ｼ繝縺壹▽譖ｴ譁ｰ・医ヶ繝ｭ繝・け縺励↑縺・ｼ・
      while (!sm.is_transitioning()) {
          battle.tick();
          
          // 繝舌ヨ繝ｫ邨ゆｺ・メ繧ｧ繝・け
          if (!battle.is_running()) {
              sm.jump(Scene::Result);
          }
          
          await(16);
      }
  });
```

**繝昴う繝ｳ繝・*:
- `tick()` 縺ｯ1繝輔Ξ繝ｼ繝縺縺大ｮ溯｡後＠縺ｦ縺吶＄謌ｻ繧・
- `run()` 縺ｨ驕輔▲縺ｦ繝悶Ο繝・け縺励↑縺・
- 繝｡繧､繝ｳ繝ｫ繝ｼ繝励〒 `battle.tick()` 繧貞他縺ｶ
- `is_running()` 縺ｧ繧ｵ繝悶せ繝・・繝医′邨ゆｺ・＠縺溘°遒ｺ隱・

---

## is_transitioning() 縺ｮ菴ｿ縺・婿

`is_running()` 縺ｯ縲後せ繝・・繝医・繧ｷ繝ｳ縺悟虚縺・※縺・ｋ縺九阪〒縺吶′縲・ 
`is_transitioning()` 縺ｯ縲形jump()` 縺悟他縺ｰ繧後※繧ｹ繝・・繝磯・遘ｻ荳ｭ縺九阪ｒ遉ｺ縺励∪縺吶・

```cpp
sm.state(Scene::Game)
  .on_update([](auto& sm) {
      while (!sm.is_transitioning()) {  // 笨・驕ｷ遘ｻ縺悟他縺ｰ繧後ｋ縺ｾ縺ｧ繝ｫ繝ｼ繝・
          // 繧ｲ繝ｼ繝繝ｭ繧ｸ繝・け
          
          if (stick(256)) {  // ESC
              sm.jump(Scene::Title);  // 竊・縺薙ｌ縺悟他縺ｰ繧後ｋ縺ｨ is_transitioning() == true
          }
          
          await(16);
      }
  });
```

**菴ｿ縺・・縺・*:
- `is_running()`: 繧ｹ繝・・繝医・繧ｷ繝ｳ蜈ｨ菴薙′蜍輔＞縺ｦ縺・ｋ縺具ｼ医Γ繧､繝ｳ繝ｫ繝ｼ繝礼畑・・
- `is_transitioning()`: 繧ｹ繝・・繝磯・遘ｻ縺悟ｧ九∪縺｣縺溘°・・on_update` 蜀・・繝ｫ繝ｼ繝礼畑・・

---

## 縺ｾ縺ｨ繧・

| 繝代ち繝ｼ繝ｳ | 逕ｨ騾・| 繝ｩ繧､繝輔し繧､繧ｯ繝ｫ |
|---------|------|--------------|
| **繝ｭ繝ｼ繧ｫ繝ｫ螟画焚** | `on_update` 蜀・□縺代〒菴ｿ縺・| 繝ｫ繝ｼ繝励・髢薙□縺・|
| **state<T>()** | `on_enter/update/exit` 縺ｧ蜈ｱ譛・| 繧ｹ繝・・繝医・髢薙□縺・|
| **Repository<T>** | 繧ｷ繝ｼ繝ｳ繧偵∪縺溘＞縺ｧ豌ｸ邯・| 繧｢繝励Μ蜈ｨ菴・|

**謗ｨ螂ｨ繝輔Ο繝ｼ**:
1. 縺ｾ縺壹Ο繝ｼ繧ｫ繝ｫ螟画焚縺ｧ譖ｸ縺・
2. `on_enter`/`on_exit` 縺悟ｿ・ｦ√↑繧・`state<T>()` 縺ｫ螟画峩
3. 繧ｷ繝ｼ繝ｳ繧偵∪縺溘＄繝・・繧ｿ縺ｯ `Repository<T>` 縺ｫ遘ｻ蜍・

---

## 髢｢騾｣鬆・岼

- [Repository API](../api/repository.html) - Repository<T> 縺ｮ隧ｳ邏ｰ
- [StateMachine API](../api/statemachine.html) - tick(), is_transitioning() 縺ｮ隧ｳ邏ｰ
