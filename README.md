# raycaster-3d

A pseudo-3D dungeon crawler built from scratch in C++ and OpenGL — no engine, no framework.

Rendering is based on the DDA (Digital Differential Analysis) raycasting algorithm,
the same technique used in *Wolfenstein 3D*. Every rendering system, texture sampler,
lighting model, and game loop is written manually from the ground up.

## Status

🚧 Active development — core systems complete, serving as engine foundation for a larger project.

## Technical Details

| | |
|---|---|
| **Language** | C++20 |
| **Graphics** | OpenGL (no engine, no framework) |
| **Renderer** | Column-based DDA raycasting with per-column texture mapping |
| **Textures** | CPU-side sampling via stb\_image — no GPU texture units used for world rendering |
| **Build system** | CMake + vcpkg |

## What's Implemented

**Rendering**
- DDA raycasting with N/S vs E/W face shading for depth perception
- Distance-based fog and shading on walls, floor, and ceiling
- Textured walls with multiple tile types (stone, brick, concrete, glass, sealed)
- Textured floor and ceiling with perspective-correct ray projection
- Billboarded sprite rendering with depth-correct z-buffer culling
- Animated sprite sheets (frame-based, configurable FPS)
- Wall-face decal system — entities rendered as overlays on specific wall faces
- Glass wall transparency — ray continues through glass to render wall behind, blended

**Lighting**
- Dynamic point light system — radius, intensity, and color per light
- Smooth inverse-square falloff with squared attenuation envelope
- Deterministic layered sine flicker — hanging bulb, wall sconce, flashlight variants
- Per-entity flicker phase offset — no two lights pulse in sync
- Light contributions accumulated per wall column and per sprite
- Player-held flashlight as a tracked light entity

**World & Entities**
- Zone manager — load/unload self-contained map zones at runtime with entry points
- Zone transition tiles — step on trigger, teleport to target zone and entry
- Dynamic map resizing — zones define their own dimensions
- Door system — horizontal and vertical orientations, recessed half-width rendering, locked variant
- Locked door registry — key item consumed on use, keyID matching for specific keys
- Entity factory system — typed prop constructors with collision, interaction, and loot flags
- Stackable entities — props have a stack top height for placing items on surfaces
- Loot containers — lockers, desks, crates with item contents, partial loot on full inventory
- Collectible pickups — key, fuse, flashlight, baton, first aid kit, logbook
- Wall note / journal system — read notes in world, collected into a persistent readable journal
- Event system — named events (`PowerOn`, `FuseRestored`, `UnlockDoor`, etc.) fire on interaction
- EventTag groups — target sets of entities by tag (e.g. all powered lights activate together)

**Player**
- Inventory system — fixed-slot array, persistent across zone transitions
- Journal — persistent entry collection, readable in-game
- Movement with sprint, stamina drain/regen, and exhaustion threshold
- Slide-along-wall collision (separate X/Y axis testing)
- Entity circle collision — player pushed out of prop collision radii
- Mouse look with pitch (vertical look) and cursor lock
- Flashlight toggle with random battery cut-out over time

**HUD**
- Crosshair
- Health and stamina bars
- Interaction prompt — context-sensitive, updates based on what you're looking at
- Notification system — timed message queue, up to 4 stacked
- Minimap — 4 modes (off, close follow, medium follow, full map)
- Inventory screen with item descriptions
- Journal screen with entry list and body text

## Controls

| Key | Action |
|---|---|
| `W / A / S / D` | Move |
| `Mouse` | Look |
| `Shift` | Sprint |
| `E` | Interact |
| `F` | Toggle flashlight |
| `I` | Inventory |
| `J` | Journal |
| `Tab` | Cycle minimap |
| `Escape` | Quit |

## Building

Requires OpenGL and CMake. Clone the repo and open `CMakeLists.txt` in Visual Studio,
or build via CMake from the command line. Dependencies are managed via vcpkg (`vcpkg.json` included).

```bash
git clone https://github.com/AEspinosa-FoxglowGames/raycaster-3d
cd raycaster-3d
cmake --preset default
cmake --build out/build/default
```

## About

Built by [Austin Espinosa](https://foxglowgames.com) — indie developer and
Information Science student at ICU Tokyo.

🌐 [foxglowgames.com](https://foxglowgames.com)

---

# raycaster-3d（日本語）

C++とOpenGLをゼロから実装した擬似3Dダンジョンクローラー。ゲームエンジン・フレームワーク不使用。

レンダリングには『Wolfenstein 3D』と同じ技術であるDDA（Digital Differential Analysis）レイキャスティングアルゴリズムを採用。レンダリングシステム、テクスチャサンプラー、ライティングモデル、ゲームループをすべて手動で実装しています。

## 開発状況

🚧 開発中 — コアシステム実装済み。より大規模なプロジェクトのエンジン基盤として使用中。

## 技術詳細

| | |
|---|---|
| **言語** | C++20 |
| **グラフィックス** | OpenGL（エンジン・フレームワーク不使用） |
| **レンダラー** | カラムベースのDDAレイキャスティング（カラム単位のテクスチャマッピング） |
| **テクスチャ** | stb\_imageによるCPU側サンプリング — ワールド描画にGPUテクスチャユニット不使用 |
| **ビルドシステム** | CMake + vcpkg |

## 実装済み機能

**レンダリング**
- N/S・E/W面の陰影差によって奥行きを表現するDDAレイキャスティング
- 壁・床・天井への距離ベースのフォグおよびシェーディング
- 複数タイルタイプ（石材、レンガ、コンクリート、ガラス、封鎖済み）に対応したテクスチャ付き壁
- 透視投影レイプロジェクションによるテクスチャ付き床・天井
- Zバッファによる深度正確なビルボードスプライト描画
- フレームベース・FPS設定可能なアニメーションスプライトシート
- ウォールフェイスデカールシステム — 特定の壁面にテクスチャオーバーレイとしてエンティティを描画
- ガラス壁の透過処理 — レイがガラスを通過して背後の壁を描画、ブレンド合成

**ライティング**
- 半径・強度・カラーを持つ動的ポイントライトシステム
- 二乗減衰エンベロープによる滑らかな逆二乗フォールオフ
- 決定論的重ね合わせサインによるフリッカー — ハンギングバルブ・ウォールスコンス・フラッシュライト各バリアント
- エンティティごとのフリッカー位相オフセット — 複数のライトが同期して点滅しない
- 壁カラムおよびスプライトへのライト寄与の蓄積計算
- トラッキングライトエンティティとしてのプレイヤー所持フラッシュライト

**ワールド・エンティティ**
- ゾーンマネージャー — エントリーポイント付きの自己完結型マップゾーンを実行時にロード・アンロード
- ゾーントランジションタイル — トリガーを踏むと目的ゾーン・エントリーへテレポート
- 動的マップリサイズ — ゾーンが独自の寸法を定義
- ドアシステム — 水平・垂直方向、フレームストリップ付き半幅凹み描画、施錠バリアント対応
- 施錠ドアレジストリ — 使用時にキーアイテムを消費、特定キーIDによるマッチング
- エンティティファクトリーシステム — コリジョン・インタラクション・ルートフラグ付きの型付きプロップコンストラクタ
- スタック可能エンティティ — プロップがスタックトップ高さを持ち、表面にアイテムを配置可能
- ルートコンテナ — ロッカー・デスク・クレートにアイテム収納、インベントリ満杯時の部分ルート対応
- 収集可能ピックアップ — 鍵、ヒューズ、フラッシュライト、警棒、救急キット、ログブック
- ウォールノート・ジャーナルシステム — ワールド内のノートを読み取り、永続的な読み取り可能ジャーナルに収集
- イベントシステム — インタラクション時に名前付きイベント（`PowerOn`・`FuseRestored`・`UnlockDoor`等）が発火
- EventTagグループ — タグによるエンティティセットの一括ターゲット（例：電源タグ付きライトが一斉点灯）

**プレイヤー**
- インベントリシステム — 固定スロット配列、ゾーン遷移をまたいで永続
- ジャーナル — 永続的なエントリー収集、ゲーム内で読み取り可能
- スプリント・スタミナ消耗/回復・疲労閾値を含む移動システム
- X軸・Y軸個別判定によるスライド壁コリジョン
- エンティティ円形コリジョン — プレイヤーがプロップのコリジョン半径から押し出される
- ピッチ（垂直視点）対応・カーソルロック付きマウスルック
- フラッシュライトトグル（時間経過によるランダムバッテリー切れあり）

**HUD**
- クロスヘア
- HPおよびスタミナバー
- インタラクションプロンプト — 視点対象に応じてコンテキスト更新
- 通知システム — 最大4件スタック可能な時間制限付きメッセージキュー
- ミニマップ — 4モード（オフ・近距離追従・中距離追従・全体表示）
- アイテム説明付きインベントリ画面
- エントリーリストと本文表示付きジャーナル画面

## 操作方法

| キー | アクション |
|---|---|
| `W / A / S / D` | 移動 |
| `マウス` | 視点操作 |
| `Shift` | スプリント |
| `E` | インタラクト |
| `F` | フラッシュライト切替 |
| `I` | インベントリ |
| `J` | ジャーナル |
| `Tab` | ミニマップ切替 |
| `Escape` | 終了 |

## ビルド方法

OpenGLとCMakeが必要です。リポジトリをクローンし、Visual Studioで `CMakeLists.txt` を開くか、コマンドラインからCMakeでビルドしてください。依存関係はvcpkgで管理されています（`vcpkg.json` 同梱）。

```bash
git clone https://github.com/AEspinosa-FoxglowGames/raycaster-3d
cd raycaster-3d
cmake --preset default
cmake --build out/build/default
```

## 開発者について

[Austin Espinosa](https://foxglowgames.com) 制作 — インディーゲーム開発者・ICU東京在籍の情報科学専攻学生。

🌐 [foxglowgames.com](https://foxglowgames.com)
