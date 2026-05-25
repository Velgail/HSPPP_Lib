---
layout: default
title: Repository API
---

# Repository API

豌ｸ邯壹ョ繝ｼ繧ｿ繧貞梛螳牙・縺ｫ邂｡逅・☆繧九Μ繝昴ず繝医Μ繝代ち繝ｼ繝ｳ縺ｮ螳溯｣・〒縺吶・

## 讎りｦ・

`Repository<T>` 縺ｨ `GameServices` 縺ｯ縲√げ繝ｭ繝ｼ繝舌Ν螟画焚縺ｮ莉｣繧上ｊ縺ｫ菴ｿ逕ｨ縺吶ｋ豌ｸ邯壹ョ繝ｼ繧ｿ邂｡逅・す繧ｹ繝・Β縺ｧ縺吶・

### 荳ｻ縺ｪ迚ｹ蠕ｴ

- 笨・**蝙句ｮ牙・**: 蜷・ョ繝ｼ繧ｿ蝙九＃縺ｨ縺ｫ迢ｬ遶九＠縺溘せ繝医Ξ繝ｼ繧ｸ
- 笨・**繧ｰ繝ｭ繝ｼ繝舌Ν繧｢繧ｯ繧ｻ繧ｹ**: 縺ｩ縺薙°繧峨〒繧ゅい繧ｯ繧ｻ繧ｹ蜿ｯ閭ｽ
- 笨・**繧ｷ繝ｳ繧ｰ繝ｫ繝医Φ**: 蝙九＃縺ｨ縺ｫ1縺､縺ｮ繧､繝ｳ繧ｹ繧ｿ繝ｳ繧ｹ
- 笨・**邁｡貎斐↑讒区枚**: `services().data<T>()`

---

## Repository<T>

### 蝓ｺ譛ｬ逧・↑菴ｿ縺・婿

```cpp
import hsppp;
using namespace hsppp;

struct PlayerData {
    int hp = 100;
    int level = 1;
    std::string name = "Player";
};

void hspMain() {
    // 繝・・繧ｿ縺ｫ繧｢繧ｯ繧ｻ繧ｹ
    auto& player = services().data<PlayerData>();
    
    player.hp -= 10;
    player.level = 5;
    
    logmes(strf("HP: %d, Level: %d", player.hp, player.level));
}
```

### 繝｡繧ｽ繝・ラ

#### `static Repository<T>& instance()`

繧ｷ繝ｳ繧ｰ繝ｫ繝医Φ繧､繝ｳ繧ｹ繧ｿ繝ｳ繧ｹ繧貞叙蠕励＠縺ｾ縺吶・

```cpp
auto& repo = Repository<PlayerData>::instance();
```

騾壼ｸｸ縺ｯ `services().repo<T>()` 縺ｾ縺溘・ `services().data<T>()` 繧剃ｽｿ逕ｨ縺励∪縺吶・

#### `T& get()` / `const T& get() const`

繝・・繧ｿ縺ｮ蜿ら・繧貞叙蠕励＠縺ｾ縺吶・

```cpp
auto& data = repo.get();
data.hp = 100;
```

#### `void set(const T& value)` / `void set(T&& value)`

繝・・繧ｿ繧定ｨｭ螳壹＠縺ｾ縺吶・

```cpp
PlayerData new_data;
new_data.hp = 50;
repo.set(new_data);

// 繝繝ｼ繝悶ｂ蜿ｯ閭ｽ
repo.set(PlayerData{});
```

#### `void reset()`

繝・・繧ｿ繧偵ョ繝輔か繝ｫ繝亥､縺ｫ繝ｪ繧ｻ繝・ヨ縺励∪縺吶・

```cpp
repo.reset();  // PlayerData{} 縺ｨ蜷後§
```

---

## GameServices

### 蝓ｺ譛ｬ逧・↑菴ｿ縺・婿

```cpp
// 繝・・繧ｿ縺ｫ逶ｴ謗･繧｢繧ｯ繧ｻ繧ｹ
auto& player = services().data<PlayerData>();

// 繝ｪ繝昴ず繝医Μ縺ｫ繧｢繧ｯ繧ｻ繧ｹ
auto& repo = services().repo<PlayerData>();
```

### 繝｡繧ｽ繝・ラ

#### `static GameServices& instance()`

繧ｷ繝ｳ繧ｰ繝ｫ繝医Φ繧､繝ｳ繧ｹ繧ｿ繝ｳ繧ｹ繧貞叙蠕励＠縺ｾ縺吶・

```cpp
auto& svc = GameServices::instance();
```

騾壼ｸｸ縺ｯ `services()` 繧ｷ繝ｧ繝ｼ繝医き繝・ヨ繧剃ｽｿ逕ｨ縺励∪縺吶・

#### `template<typename T> Repository<T>& repo()`

謖・ｮ壹＠縺溷梛縺ｮ繝ｪ繝昴ず繝医Μ繧貞叙蠕励＠縺ｾ縺吶・

```cpp
auto& repo = services().repo<PlayerData>();
```

#### `template<typename T> T& data()`

繝ｪ繝昴ず繝医Μ縺ｮ繝・・繧ｿ縺ｫ逶ｴ謗･繧｢繧ｯ繧ｻ繧ｹ縺励∪縺吶・

```cpp
auto& player = services().data<PlayerData>();
player.hp -= 10;
```

#### `template<typename T, typename Tag = GlobalTag> void register_repository()`

繝ｪ繝昴ず繝医Μ繧堤匳骭ｲ縺励∪縺呻ｼ・reset_all()` 縺ｧ荳諡ｬ繝ｪ繧ｻ繝・ヨ蜿ｯ閭ｽ縺ｫ縺吶ｋ・峨・

```cpp
services().register_repository<PlayerData>();  // 譌｢螳壹・ GlobalTag
services().register_repository<GameProgress>();

// 繧ｿ繧ｰ繧呈欠螳壹＠縺ｦ逋ｻ骭ｲ・井ｻｻ諢上・蝙九〒OK・・
services().register_repository<PlayerData, StateTag>();

// 繝ｦ繝ｼ繧ｶ繝ｼ螳夂ｾｩ繧ｿ繧ｰ繧ゆｽｿ逕ｨ蜿ｯ閭ｽ
struct SaveSlot1Tag : RepositoryTagBase {};
services().register_repository<PlayerData, SaveSlot1Tag>();
```

#### `void reset_all()`

逋ｻ骭ｲ縺輔ｌ縺溘☆縺ｹ縺ｦ縺ｮ繝ｪ繝昴ず繝医Μ繧偵Μ繧ｻ繝・ヨ縺励∪縺吶・

```cpp
// 繧ｲ繝ｼ繝髢句ｧ区凾縺ｫ蜈ｨ繝・・繧ｿ繧偵Μ繧ｻ繝・ヨ
services().reset_all();
```

---

## services() 繝倥Ν繝代・髢｢謨ｰ

`GameServices::instance()` 縺ｮ繧ｷ繝ｧ繝ｼ繝医き繝・ヨ縺ｧ縺吶・

```cpp
// 縺薙・2縺､縺ｯ蜷後§
auto& data1 = GameServices::instance().data<PlayerData>();
auto& data2 = services().data<PlayerData>();
```

---

## 螳溯ｷｵ萓・

### 隍・焚縺ｮ繝・・繧ｿ蝙九ｒ邂｡逅・

```cpp
struct PlayerProfile {
    std::string name = "Player";
    int high_score = 0;
};

struct GameSettings {
    int difficulty = 1;
    float volume = 0.8f;
};

void hspMain() {
    // 繝ｪ繝昴ず繝医Μ繧堤匳骭ｲ
    services().register_repository<PlayerProfile, StateTag>();
    services().register_repository<GameSettings, GlobalTag>();
    
    // 繝・・繧ｿ縺ｫ繧｢繧ｯ繧ｻ繧ｹ
    auto& profile = services().data<PlayerProfile>();
    auto& settings = services().data<GameSettings>();
    
    profile.high_score = 1000;
    settings.volume = 0.5f;
    
    // 蜈ｨ繝ｪ繧ｻ繝・ヨ
    services().reset_all();
    
    logmes(strf("High Score: %d", profile.high_score));  // 0
}
```

### StateMachine 縺ｨ縺ｮ邨・∩蜷医ｏ縺・

```cpp
enum class Scene { Title, Game, Result };

void hspMain() {
    StateGraph<Scene> sm;
    
    sm.state(Scene::Game)
      .on_update([](auto& sm) {
          // 笨・繧ｭ繝｣繝励メ繝｣荳崎ｦ√〒繧｢繧ｯ繧ｻ繧ｹ
          auto& profile = services().data<PlayerProfile>();
          
          while (!sm.is_transitioning()) {
              profile.high_score += 10;
              
              mes(strf("Score: %d", profile.high_score));
              await(16);
          }
      });
    
    sm.start(Scene::Title);
}
```

---

## 繝吶せ繝医・繝ｩ繧ｯ繝・ぅ繧ｹ

### 笨・謗ｨ螂ｨ

1. **讒矩菴薙〒繝・・繧ｿ繧偵げ繝ｫ繝ｼ繝怜喧**
   ```cpp
   struct PlayerData {
       int hp;
       int mp;
       int level;
   };
   ```

2. **繝・ヵ繧ｩ繝ｫ繝亥､繧定ｨｭ螳・*
   ```cpp
   struct GameSettings {
       int difficulty = 1;
       float volume = 0.8f;
   };
   ```

3. **services() 繧剃ｽｿ逕ｨ**
   ```cpp
   auto& data = services().data<PlayerData>();  // 笨・
   auto& data = Repository<PlayerData>::instance().get();  // 笶・蜀鈴聞
   ```

### 笞・・豕ｨ諢冗せ

1. **繧ｰ繝ｭ繝ｼ繝舌Ν螟画焚縺ｨ蜷檎ｭ・*: 繝・せ繧ｿ繝薙Μ繝・ぅ縺ｯ菴弱＞
2. **蝙九＃縺ｨ縺ｫ1縺､**: 蜷後§蝙九〒隍・焚縺ｮ繧､繝ｳ繧ｹ繧ｿ繝ｳ繧ｹ縺ｯ菴懊ｌ縺ｪ縺・
3. **繧ｹ繝ｬ繝・ラ螳牙・諤ｧ縺ｪ縺・*: 繝槭Ν繝√せ繝ｬ繝・ラ縺ｧ縺ｯ菴ｿ逕ｨ荳榊庄

---

## 髢｢騾｣鬆・岼

- [StateMachine API](statemachine.html) - 繧ｹ繝・・繝医・繧ｷ繝ｳ縺ｨ縺ｮ騾｣謳ｺ
- [Data Sharing Guide](../guides/data-sharing.html) - 繝・・繧ｿ蜈ｱ譛峨・繝吶せ繝医・繝ｩ繧ｯ繝・ぅ繧ｹ
