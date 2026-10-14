# Maze Chase codebase report

All behavior below is described from the checked-in source as read for this report. File-and-line citations use the current source line numbers. A value shown in `Config.h` is not necessarily the only literal used for that visual feature; the report calls out important literals that remain inline. No source files were changed to prepare this report.

## 0. SNAPSHOT

### File tree and line counts

`wc -l`-style counts (PowerShell line counts were used on this Windows workspace; they are equivalent to newline-count line totals for these files):

```text
src/
  Config.h                         124
  DirectionInput.cpp                61
  DirectionInput.h                  48
  Game.cpp                        1506
  Game.h                           198
  GridMover.cpp                    129
  GridMover.h                       54
  Hunter.cpp                       150
  Hunter.h                          43
  Leaderboard.cpp                   85
  Leaderboard.h                     36
  Maze.cpp                         135
  Maze.h                            57
  Pathfinder.cpp                   134
  Pathfinder.h                      19
  Player.cpp                       130
  Player.h                          36
  RenderUtils.h                    145
  Theme.h                           21
  UI.cpp                           160
  UI.h                              41
  dsa_lib/
    BST.h                          269
    Queue.h                        111
    Stack.h                        119
CMakeLists.txt                      52
```

Within the requested scope there are 28 files: 24 under `src/` (21 application files and 3 custom DSA headers), one CMake file, and three binary font assets. `src/` has 3,832 lines; CMake has 52, for 3,884 source/build-description lines total. By the requested functional grouping: custom DSA = 499 lines; game logic = 2,842 lines; rendering/UI = 367 lines; config = 124 lines. These groups do not include the separate 52-line CMake file. Binary assets have no meaningful `wc -l` count: `assets/fonts/Orbitron-Variable.ttf` (38,576 bytes), `Rajdhani-Bold.ttf` (400,680 bytes), `Rajdhani-Medium.ttf` (384,348 bytes).

The requested comparison says the project grew from 8 files. The present tree is countable, but no 8-file baseline or history was provided, and the repository rules prohibit Git inspection. Therefore the exact list of files added since that baseline is **unsure**. The current set contains separate files for custom data structures, maze/pathfinding, player/hunter movement, UI, rendering helpers, leaderboard, and input handling; those are plausible later additions, not a verified before/after list.

### Build system and dependencies

`CMakeLists.txt` requires CMake 3.16 and C++17; it uses FetchContent to obtain SFML 2.6.1, disables SFML audio/network/examples/docs, builds executable `MazeChase` from ten `.cpp` files, and links graphics/window/system (CMakeLists.txt:1-52). The three custom data structures are header-only. Compiler warnings include `-Wall -Wextra`. On Windows, a post-build step copies SFML DLLs and the `assets` directory next to the executable. This report did not run a full build.

## 1. BIG PICTURE

### Launch to quit

1. `main` accepts an optional `--seed <number>`, then constructs `Game` and calls `run` (src/main.cpp:4-21).
2. `Game` creates a 1280×720 SFML window, sets a 60 FPS limit, creates views/menu items, loads fonts, builds a radial gradient texture, and begins a generated single-player maze (src/Game.cpp:19-88).
3. The game loop measures frame time, processes window events, updates state, renders, and displays the frame (src/Game.cpp:247-258).
4. In the menu, the player selects single-player, two-player, leaderboard, instructions, controls test, or quit. A new game enters `Generating`; after generation it makes a cached maze texture and enters a preview (src/Game.cpp:286-317, 170-219).
5. Gameplay updates movement, fog, power-ups, timers, hunter, and outcome checks. Escape/pause, catch, escape through the exit, timeout, level completion, and name entry change the current state (src/Game.cpp:318-445, 447-748).
6. Closing the SFML window ends `run` and returns from `main`; there is no separate save-on-quit hook. Leaderboard additions save at the time a name is submitted (src/Game.cpp:383-394; src/Leaderboard.cpp:64-68).

### State machine

All states are declared in `GameState` (src/Game.h:14-28). `MenuSubScreen`, roles, outcomes and power-up kinds are adjacent (src/Game.h:30-59).

| State | Entry/exit trigger | Update and drawing behavior |
|---|---|---|
| `Menu` | Initial state; return from menu/help/leaderboard; menu selection starts a game or opens a screen. | Animated menu background and selected screen; no maze gameplay update. Main menu uses `MenuList`; submenu Esc/Enter returns to main. Main-menu Esc itself is not handled as quit (src/Game.cpp:286-317, 447-472, 750-807). |
| `Generating` | New single-player game/retry/next level or start/restart 2P match/round. | Calls incremental maze steps each update; Space loops to completion immediately. Draws generation visualizer. When done, configures entities and enters preview (src/Game.cpp:337-348, 447-485, 1453-1476). |
| `Preview` | Single-player generation finishes. | Maze preview; gameplay movement is frozen for `MOVEMENT_FREEZE_TIME` and the preview expires after `MAP_PREVIEW_DURATION`, then `Playing` (src/Game.cpp:447-605). |
| `Playing` | Preview expires or pause resumes. | Full-screen shared player-view renderer; runner movement, fog, powerups; hunter begins after its wake period. Exit or catch completes the run/level (src/Game.cpp:447-605, 750-872). |
| `Paused` | Esc during SP preview/playing; resume/retry/menu from pause list. | Pause screen and no normal update. F3 can toggle BFS demo. Resume returns to `Playing` even if entered from preview (src/Game.cpp:349-366, 447-472, 1415-1425). |
| `GameOver` | SP catch or other terminal run condition; returning from name entry if no pending winner name. | Draws the end screen/menu. Retry resets level/run score and starts generation; menu returns to `Menu` (src/Game.cpp:367-377, 1427-1451). |
| `LevelCleared` | SP reaches exit. | Draws cleared screen; Enter/Space advances level and generates a new maze (src/Game.cpp:378-382, 1470-1476). Important: current `update` does not early-return for this state; see Risks. |
| `NameEntry` | SP catch ends a run, or round two completes a 2P match with a winner to record. | Text input and submission overlay. Enter adds to leaderboard and returns to game-over or match-over (src/Game.cpp:383-395, 1478-1485). Important: current `update` falls through to SP updates here too. |
| `TwoPlayerPreview` | A generated 2P round is ready. | Shared view renderer called twice, one half per role; runner/hunter release timers run, then transitions to `TwoPlayerPlaying` (src/Game.cpp:236-245, 607-748, 965-984). |
| `TwoPlayerPlaying` | 2P preview release expires or pause resumes. | Two players move by player-bound controls; round timer/outcome checks and role-aware views run (src/Game.cpp:396-411, 607-748). |
| `TwoPlayerPaused` | Esc during active 2P. | Pause UI. Resume, restart same match seed, or main menu (src/Game.cpp:412-423, 986-1033). |
| `TwoPlayerIntermission` | Round one ends. | Shows round-one result; Enter/Space starts round two using same maze seed, Esc returns to menu (src/Game.cpp:424-433, 986-1033). |
| `TwoPlayerMatchOver` | Round two ends. | Aggregates both round scores, tie-breaks by escape times, asks winner for name; rematch/menu options (src/Game.cpp:434-445, 1035-1089). |

`MenuSubScreen` has Main, Leaderboard, and HowToPlay; it is a menu submode rather than a `GameState`. `PlayerRole` is Runner/Hunter, `RoundOutcome` is Escape/Caught/TimeOut, and `PowerUpType` is None/Surge/Dash/Ping (src/Game.h:30-59).

### Exact per-frame order

`Game::run` restarts a clock for `dt`, clamps it to `cfg::DELTA_TIME_CAP`, then calls `processEvents()`, `update(dt)`, and `render()` in that order (src/Game.cpp:247-258). `render()` clears, selects a screen/world rendering branch, draws the controls overlay if enabled, then calls `window_.display()` (src/Game.cpp:750-807). Events therefore affect the same frame's update and draw. The update first maintains average FPS/auto-Low-FX/toast timing, then handles test overlay/menu/generation/pause/terminal early returns; otherwise it performs SP gameplay or dispatches 2P update (src/Game.cpp:447-605).

## 2. FILE-BY-FILE DEEP DIVE

### `src/Config.h`

Defines namespace `cfg`, all named tunables, compile-time odd-maze assertions and shared colors (src/Config.h:1-124). It has no classes/functions. Consumers include Game, Maze, Player, GridMover, Hunter, RenderUtils, Theme, Pathfinder (directly or through headers). Some actual layout/drawing numbers remain literals in `Game.cpp` and `RenderUtils.h`; they are not all centralized.

### `src/DirectionInput.h` and `.cpp`

`KeySet` groups four SFML keys; `Direction` contains row/column delta and `isZero`; `DirectionInput` owns a key set, four previous-key flags, press-order vector, current direction, and a global press tick (src/DirectionInput.h:6-48). Constructor stores keys and initializes state (src/DirectionInput.cpp:4-6). `reset(window)` clears history and snapshots keys currently held, so a key already down during reset does not become a fresh press (src/DirectionInput.cpp:8-16). `update(window)` checks focus and physical key state, updates press/release history and chooses the most recently pressed still-held direction; it returns row/column movement (src/DirectionInput.cpp:18-61). Accessors expose direction, press tick, held keys, individual key state and a printable direction string (src/DirectionInput.h:25-47). Used by Game to make last-key-pressed movement and to preserve player-bound key sets. Fragile edge: on focus loss it returns zero but does not clear press history/previous-key flags (src/DirectionInput.cpp:18-23).

### `src/Game.h` and `.cpp`

`Game` is the coordinator for SFML window, state machine, maze, entities, inputs, camera/view setup, cached textures, fonts, powerups, scores, leaderboard, menus, and rendering (src/Game.h:61-198). Its functions are listed below; signatures and related fields are in the cited header ranges.

- Constructor initializes the 1280×720 window, views, menu choices, resources and starting round; `run` is the event/update/render loop (src/Game.cpp:19-88, 247-258).
- `buildRadialGradient` creates a 128×128 radial alpha texture; `rebuildMazeTexture` draws the maze once to an `sf::RenderTexture`; `tryLoadFont` tries a font path and fallbacks; `loadFonts` provides project/system paths (src/Game.cpp:90-157).
- `resetRound` starts SP generation and resets state; `finishGeneratedRound` creates texture/entities/spawns/powerups, sets level fog and hunter speed; `startTwoPlayerMatch` chooses/reuses a seed and resets totals; `startTwoPlayerRound` resets input/BFS and generates that match's maze (src/Game.cpp:159-245).
- `processEvents` handles window close, global toggles, menu navigation, state-specific keys, text entry and selection actions (src/Game.cpp:260-445). `update` updates timers/FPS, generation, pause gates and SP rules; `updateTwoPlayer` updates two-player timer, controls, roles and outcome scoring (src/Game.cpp:447-748).
- `render` dispatches screen branches; `drawPlayerView(rect, role, ...)` is the shared renderer called once for SP and twice for 2P; `renderFog`, `renderExitPortal`, and power-up render methods draw world overlays (src/Game.cpp:750-984).
- `spawnPowerUps`, `activatePowerUp`, `updatePowerUps`, `renderPowerUps` place/activate/tick/draw powerups (src/Game.cpp:909-963).
- `renderTwoPlayerIntermission` and `renderTwoPlayerMatchOver` draw round screens and determine/display winner (src/Game.cpp:986-1089).
- `renderControlsTestOverlay`, `drawPlayerHud`, `renderMenu`, `renderLeaderboard`, `renderHowToPlay`, `renderPause`, `renderGameOver`, `renderGeneration`, `renderNameEntry`, `renderBfsDemo` respectively draw those overlays/screens and HUD (src/Game.cpp:1091-1506).

The state fields include `state_`, `subScreen_`, role/outcome, timers, level/run score, two-player round data and seed, overlay toggles, and name-entry buffer; entity fields distinguish SP runner/hunter from 2P runner/hunter (src/Game.h:110-198). Camera fields keep one SP view and two 2P views/positions. Texture/font fields keep the prerendered maze, radial gradient and three font choices. Power-up fields store map pickups, per-player held charge and timer. Menu fields hold main/pause/2P/leaderboard choices. The source has no particle pool or screen-shake state.

### `src/Maze.h` and `.cpp`

`Maze` stores rows/columns, integer wall/path grid, RNG/seed, custom DFS stack, braid candidate list/index/count, generation phase/current cell/action text, and counters (src/Maze.h:8-57). Constructor builds dimensions/grid and asserts dimensions are odd; `inBounds` checks coordinate bounds; `isWall` treats out-of-bounds as a wall (src/Maze.cpp:7-22). `beginGenerate(seed)` fills walls, selects a nonzero random seed if passed zero, initializes the RNG and starts DFS at `(1,1)` (src/Maze.cpp:24-53). `stepGenerate()` performs one DFS push or pop, then incremental braid removals, returns whether work remains; however, the transition into braid scans and shuffles all maze cells in one call (src/Maze.cpp:55-130). `generate(seed)` runs the same begin/step implementation to completion (src/Maze.cpp:132-135). Accessors expose cells, fixed start/exit, phase/action/current cell/counters/seed and top stack snapshot (src/Maze.h:20-56). No minimum dimension validation beyond oddness; the fixed `(1,1)` and `(rows-2,cols-2)` assume adequate dimensions.

### `src/Pathfinder.h` and `.cpp`

`BfsResult` contains `found`, `path`, `visitOrder`, `nodesVisited`, `maxQueueSize`, and elapsed microseconds; `Pathfinder` provides `findPath` and `findSpawnCell` (src/Pathfinder.h:5-19). `findPath` validates endpoints, performs iterative four-neighbor BFS using `dsa_lib::Queue`, records each dequeued cell, parents, visited count and max queue, then reconstructs the path backwards and reverses it (src/Pathfinder.cpp:11-91). Start equal to goal is a special successful one-cell path. `findSpawnCell` BFSes reachable paths from the runner start and chooses an open cell at the requested distance if possible (src/Pathfinder.cpp:94-134). Both are O(V+E) time and O(V) auxiliary memory for V maze cells and E orthogonal neighbor links. Unreachable target returns no found path; spawn selection falls back to the start if no candidate is found.

### `src/GridMover.h` and `.cpp`

`GridMover` holds logical current/target row and column, floating pixel position, facing delta, step count, moving flag, cell width/height, cells-per-second, speed multiplier, snap threshold and animation timing/frame (src/GridMover.h:5-54). Constructor places the mover at a cell center and initializes speed/animation from Config (src/GridMover.cpp:4-20). `setTargetCell` sets target/facing/moving and increments steps for a distinct target; it does not itself verify adjacency or walls (src/GridMover.cpp:22-30). `updateWithInput` advances an existing segment, snaps to cell center, then when centered reads one input direction and starts a move if the neighboring cell is open; there is no buffered turn queue (src/GridMover.cpp:32-89). `updateTowardsTarget` advances a previously selected target and reports arrival (src/GridMover.cpp:91-127). Getters/setters are inline in the header (src/GridMover.h:11-32). Pixel movement is `cells/sec × multiplier × cell width`; snapping corrects small float residue, but leftover dt after reaching a cell is not carried into the next segment.

### `src/Player.h` and `.cpp`

`Player` wraps `GridMover` and two `vector<vector<bool>>` maps: remembered/discovered and currently visible (src/Player.h:7-36). Constructor initializes the mover; `update` forwards movement; `resetDiscovery` sizes both maps (src/Player.cpp:6-21). `discoverAround` clears only current visibility, preserves discovered history, marks the player's cell, casts sampled rays within a circular radius, and adds a forward cone when facing a direction (src/Player.cpp:23-118). A diagonal ray is stopped when both orthogonal side cells are walls; the blocking wall itself can be marked before the ray ends (src/Player.cpp:64-82). `isDiscovered` and `canSeeNow` safely return false outside valid map cells (src/Player.cpp:120-130). This is approximate sampled grid LOS, not an exact geometry algorithm.

### `src/Hunter.h` and `.cpp`

`Hunter` wraps a `GridMover`, remembered/current visibility maps, latest BFS result and a BFS revision counter (src/Hunter.h:8-43). `update` does nothing before wake; when centered it runs BFS to the player, sets the next path cell as target, advances movement, then recalculates at the next decision point (src/Hunter.cpp:14-45). Thus AI plans are refreshed at cell decisions, not on a fixed timer. `updateHuman` forwards movement only if released (src/Hunter.cpp:47-54). `resetDiscovery`, `discoverAround`, `isDiscovered`, and `canSeeNow` mirror the Player map/LOS behavior but have no forward light cone (src/Hunter.cpp:56-150). Accessors/setters expose mover state and latest path; in human mode the AI BFS is not being refreshed by `updateHuman`.

### `src/Leaderboard.h` and `.cpp`

`LeaderboardEntry` stores name, integer score, mode, detail and insertion ID; `LeaderboardCompare` defines BST ordering; `Leaderboard` owns a custom BST and next ID (src/Leaderboard.h:6-36). The comparator puts lower score to the left and for equal scores puts lexicographically larger names/IDs left, so reverse-inorder produces descending score and ascending name/ID (src/Leaderboard.cpp:9-13). Constructor loads; `filePath` chooses executable sibling on Windows or relative `leaderboard.txt` otherwise; `load` parses TSV and inserts in read order; `save` writes preorder; `add` sanitizes name line delimiters, truncates to ten characters, assigns ID and saves (src/Leaderboard.cpp:15-68). `top` collects reverse-inorder entries up to a limit; `traversal` selects in/pre/post order (src/Leaderboard.cpp:71-85). Details are not sanitized for tabs/newlines; this can corrupt TSV structure.

### `src/UI.h` and `.cpp`

`UI.h` declares `drawText`, `drawPanel`, `drawAnimatedBackground`, `MenuItem`, and `MenuList` (src/UI.h:8-41). `drawText` configures text, origin/centering and draws; `drawPanel` draws a rectangle and optional outline; animated background draws the moving grid (src/UI.cpp:8-57). `MenuList::addItem` stores label/enabled/id; `navigate` wraps while skipping disabled items; `handleEvent` maps W/Up, S/Down, Enter/Space, Esc to navigation/selection codes; `setSelectedIndex` accepts only valid enabled options; `draw` lays out panels and labels (src/UI.cpp:59-160). `MenuItem` fields are label, enabled, id; `MenuList` stores vector items, selected index and layout position (src/UI.h:17-40). Menu IDs are interpreted by Game rather than the UI component.

### `src/Theme.h` and `src/RenderUtils.h`

`Theme.h` defines inline color/layout conveniences sourced from Config (src/Theme.h:1-21). `RenderUtils.h` contains inline `drawAnimatedPlayer` and `drawAnimatedHunter`; both compose simple SFML circles/rectangles, use frame/facing/movement/pulse inputs, and draw directly to a window (src/RenderUtils.h:7-145). The “sprites” are geometric shapes, not image assets. Player helper animates limbs/head and moving trail; hunter helper animates claws/robe and uses a dormant visual. Their geometric measurements are inline literals, not Config entries.

### `src/main.cpp`

`main(argc, argv)` scans for `--seed`, parses following text with `stoul` inside a catch-all, constructs `Game(initialSeed)`, invokes `run`, and returns zero (src/main.cpp:4-21). Invalid seed text silently leaves the default/previous seed. There are no other command-line options.

### `src/dsa_lib/Stack.h`, `Queue.h`, `BST.h`

These are templated, header-only custom linked structures and their detailed algorithm analysis is in Section 3. `Stack` owns `topNode/count`; `Queue` owns `headNode/tailNode/count`; `BST` owns root/count/comparator and recursive helpers. Their rule-of-five operations copy nodes or transfer pointers (Stack.h:10-119; Queue.h:9-111; BST.h:12-269). DSA CORE comments are present on the custom structure code. No `std::stack`, `std::queue`, or `std::map` is used for these jobs.

### `CMakeLists.txt` and `assets/`

CMake defines the C++17/SFML build described in Snapshot (CMakeLists.txt:1-52). `assets/fonts/Orbitron-Variable.ttf`, `Rajdhani-Bold.ttf`, and `Rajdhani-Medium.ttf` are font files only; no image/sound assets are needed for current rendering. Runtime path behavior and font fallback are in Section 6. Asset binaries have no meaningful `wc -l` count.

## 3. DSA SECTION

### Custom stack

`dsa_lib::Stack<T>` is a singly linked LIFO list: each node contains `T data` and `Node* next`; the object owns a top pointer and count (src/dsa_lib/Stack.h:10-20). `push` prepends, `pop` deletes top if nonempty, `top` asserts nonempty, `empty/size` inspect metadata, `clear` repeatedly pops, and `topEntries(n)` copies at most n top-down values without mutation (src/dsa_lib/Stack.h:68-117). Rule of five: destructor clears; copy constructor duplicates in original order; copy assignment uses temporary-and-swap; move constructor/assignment transfer ownership (src/dsa_lib/Stack.h:22-65). Push/pop/top are O(1); size/empty O(1); clear/copy O(n); topEntries O(min(n,size)) time and output space. Maze DFS uses it as the active carving path; generation UI uses `topEntries(12)` for a visualization snapshot (src/Maze.h:34-38; src/Game.cpp:1453-1476).

### Custom queue

`dsa_lib::Queue<T>` is a singly linked FIFO with head, tail and count (src/dsa_lib/Queue.h:9-20). `push` links at tail, `pop` removes head and clears tail when last node is removed, `front` asserts nonempty, `empty/size` are constant-time, and `clear` drains (src/dsa_lib/Queue.h:66-109). Copy/move/destruction are implemented (src/dsa_lib/Queue.h:22-63). Enqueue/dequeue/front are O(1); clear/copy O(n). Pathfinder uses it as BFS frontier (src/Pathfinder.cpp:25-68, 103-126).

### Binary search tree and leaderboard ordering

`dsa_lib::BST<T,Compare>` nodes own a value and raw left/right pointers; tree owns root/count/comparator (src/dsa_lib/BST.h:11-24). It is an ordinary unbalanced binary search tree—there are no rotations or balancing metadata.

- `insert` recursively compares; comparator-true goes left, comparator-false in reverse goes right. If neither compares less (equivalent key), it assigns the new value to that node and does not add a node (src/dsa_lib/BST.h:41-55, 183-185). This means duplicate score/name/id keys overwrite, not coexist.
- `remove` searches recursively. It deletes a leaf directly; replaces a one-child node with its child; for two children it copies the smallest value in the right subtree (inorder successor) then removes that successor node (src/dsa_lib/BST.h:57-98, 187-191). Return value says whether a node was removed.
- `search` iteratively follows comparator order and returns a pointer to stored value or null; `contains` wraps it (src/dsa_lib/BST.h:193-209).
- Inorder is left/node/right; preorder node/left/right; postorder left/right/node; reverse-inorder right/node/left. Callback receives each value (src/dsa_lib/BST.h:108-137, 244-261).
- `height` recursively computes max child height plus one; empty tree gives zero (src/dsa_lib/BST.h:100-106, 219-221). `min` and `max` follow leftmost/rightmost pointers and assert the tree is not empty (src/dsa_lib/BST.h:229-242). `size`, `empty`, and `clear` use count or recursive destruction (src/dsa_lib/BST.h:211-227).
- Rule-of-five copy recursively clones nodes, move transfers root/count, destructor/clear destroy recursively (src/dsa_lib/BST.h:26-39, 139-179).

Insert/search/remove/min/max cost O(h), where h is height; traversals/height/destruction/copy cost O(n), recursive stack O(h). Balanced average h is logarithmic, but sorted/adversarial insertion can make h=n, yielding O(n) operations and recursion depth n; stack overflow is possible for very large skewed trees. The game does not rebalance.

`LeaderboardCompare` orders score ascending for the tree, then name descending, then insertion ID descending in ordinary inorder; reversing that order produces the requested visible order: score high-to-low, name A-to-Z, ID low-to-high (src/Leaderboard.cpp:9-13). When there are fully equal fields including ID, BST overwrites the equivalent value.

### DFS maze generation and braiding

`Maze::beginGenerate` fills grid with walls and pushes `(1,1)` as the first DFS cell; `stepGenerate` inspects the top cell, gathers still-walled neighbors two cells away, chooses one pseudorandomly, opens the intervening wall and destination, and pushes the destination. If no neighbors remain it pops and records a backtrack action (src/Maze.cpp:24-114). This is iterative recursive-backtracker DFS. Each carved cell is pushed once and eventually popped once: O(V) grid work for the DFS portion, O(V) stack storage worst case. Generation UI calls a bounded number of steps per frame; `generate(seed)` loops over precisely the same step implementation for synchronous generation (src/Maze.cpp:132-135; src/Game.cpp:447-485).

After DFS, the implementation scans wall cells for candidate connectors whose opposite neighbors are open while perpendicular neighbors are walls, shuffles them, and removes `floor(candidateCount * BRAID_PERCENT)` walls one per later `stepGenerate` call (src/Maze.cpp:63-80, 116-129). This reduces dead ends/loops into the maze. The full scan and shuffle happen together during the transition call, so “one step” is not a strict constant-time unit there. Candidate selection/shuffle uses the maze RNG; random selection uses modulo `rng()%options.size()`, which can have small modulo bias.

Live demonstration: start a game and observe `Generating`; stack snapshot, current cell, action, phase and counters are drawn by `Game::renderGeneration` (src/Game.cpp:1453-1476). Space completes it immediately; +/- changes step rate (src/Game.cpp:337-348).

### BFS pathfinding and hunter decision loop

`Pathfinder::findPath` uses a visited matrix, parent matrix and custom FIFO queue; each open cell is enqueued at most once and has at most four edges (src/Pathfinder.cpp:11-91). Complexity O(V+E), O(V) space; on this grid E≤4V, effectively O(V). Parent links reconstruct shortest path in number of maze steps. It also captures pop order and queue metrics for display. `findSpawnCell` similarly BFSes from start, recording distances and choosing the reachable cell nearest target distance (src/Pathfinder.cpp:94-134).

`Hunter::update` requests a new BFS when the hunter is centered, selects `path[1]`, advances toward that target and requests another plan at the next cell (src/Hunter.cpp:14-45). It is called each active update frame, but BFS itself runs at a cell decision, not every frame. On a 39×39 map each search is bounded and small, though the worst-case scan remains O(V). Latest result/revision feeds the optional SP BFS overlay; its visit wave is displayed over about 1.2 seconds and then its final route (src/Game.cpp:1487-1506; src/Config.h:39).

### Fog, leaderboard persistence and other algorithms

Player and hunter LOS each ray-sample target cells and update visible/discovered bool grids (src/Player.cpp:23-118; src/Hunter.cpp:61-138). The rendering overlay consumes those grids; fog state belongs to each entity, not to the renderer.

Leaderboard load inserts TSV records in file order. Save traverses preorder, which preserves the tree's shape when records are reinserted in that same sequence (src/Leaderboard.cpp:32-61). Shape matters because this BST is unbalanced and insertion order determines its shape. Preorder is root-left-right, so reinserting a preorder sequence reconstructs the same ordered shape assuming comparator and record data are unchanged. Top 10 uses reverse-inorder (src/Leaderboard.cpp:71-85; src/Game.cpp:1332-1373).

Live DSA demonstration: generate a maze to show Stack push/pop; F3 during SP play/pause shows BFS visit order/path and metrics; leaderboard screen shows tree node count/height/top levels and keys 1/2/3 choose traversal displays (src/Game.cpp:1332-1373, 1487-1506). BST remove, min and max are public API but are not invoked by the game UI/runtime.

## 4. GAME MECHANICS SPEC

### Config.h constants

Values below are directly defined in `src/Config.h`; meanings are based on call sites and names. Colors are RGB or RGBA constructor values. See the file itself for exact definitions and grouping (src/Config.h:7-122).

| Constant | Value | Meaning in code |
|---|---:|---|
| `WINDOW_WIDTH`, `WINDOW_HEIGHT`, `TARGET_FPS` | 1280, 720, 60 | Window dimensions and frame cap. |
| `MAZE_ROWS`, `MAZE_COLS`, `TILE_SIZE` | 39, 39, 32 | Grid size and world pixels per cell. |
| `CAMERA_VIEW_W`, `CAMERA_VIEW_H`, `CAMERA_LERP_SPEED` | 800, 450, 7 | SP view size and exponential follow rate. |
| `MINIMAP_SIZE`, `MINIMAP_PADDING` | 160, 20 | Minimap extent and edge inset. |
| `MENU_BG_GRID_SPACING`, `MENU_BG_SPEED` | 40, 12 | Animated menu grid. |
| `WALL_TILE`, `PATH_TILE`, `STEP_DIST`, `BRAID_PERCENT`, `NUM_DIRECTIONS` | 1, 0, 2, .18, 4 | Grid encoding, DFS jump, braid fraction, cardinal directions. |
| `GENERATION_STACK_VISIBLE`, `BFS_DEMO_WAVE_SECONDS` | 12, 1.2 | Generation stack snapshot cap; BFS visit animation duration. |
| `LOW_FX_FPS_THRESHOLD`, `LOW_FX_TRIGGER_SECONDS` | 45, 3 | Auto-Low-FX threshold and sustained duration. |
| `DANGER_DISTANCE_TILES`, `PORTAL_ROTATION_SPEED`, `RADIAL_GRADIENT_SIZE` | 11, 70, 128 | Danger proximity scale, portal rotation rate, gradient texture side length. |
| `POWERUP_RESPAWN_SECONDS`, `SURGE_DURATION`, `DASH_DURATION`, `PING_DURATION` | 10, 2, 1, 2 | Pickup respawn and effect durations. |
| `SURGE_MULTIPLIER`, `DASH_MULTIPLIER` | 1.6, 1.8 | Movement speed multipliers. |
| `GENERATION_DFS_STEPS_PER_FRAME`, `GENERATION_BRAID_STEPS_PER_FRAME` | 6, 1 | Base visualizer work per frame by phase. |
| `Background`, `Panel`, `Cyan`, `Crimson`, `Green`, `Amber`, `Text`, `TextDim`, `TextMuted`, `PanelOutline`, `OverlayDim`, `MenuGrid`, `WallBase`, `PathBase` | `(7,10,18)`, `(14,20,34)`, `(0,229,255)`, `(255,45,85)`, `(0,255,135)`, `(255,184,0)`, `(230,236,255)`, `(140,155,190)`, `(95,105,135)`, `(30,48,80)`, `(7,10,18,220)`, `(15,25,45,75)`, `(18,23,38)`, `(10,13,24)` | Shared UI/maze palette. |
| `PLAYER_CELLS_PER_SEC`, `HUNTER_CELLS_PER_SEC` | 7, 6 | Base SP runner and AI hunter speed. |
| `SNAP_THRESHOLD`, `HUNTER_SNAP_THRESHOLD` | 1, 1 px | Cell-center snapping. |
| `ANIM_FRAME_TIME`, `TOTAL_ANIM_FRAMES`, `HUNTER_ANIM_FRAME_TIME`, `HUNTER_TOTAL_FRAMES` | .10, 4, .10, 4 | Movement animation timing/count. |
| `MAP_PREVIEW_DURATION`, `MOVEMENT_FREEZE_TIME`, `HUNTER_WAKE_TIME`, `DELTA_TIME_CAP` | 5 s, 4 s, 8 s, .1 s | Preview duration, opening movement freeze, SP hunter wake, frame dt clamp. |
| `HUNTER_BLINK_CYCLE`, `HUNTER_BLINK_ON_TIME` | 1.5 s, .25 s | SP hunter blink cadence/visible period. |
| `FOG_VISION_RADIUS`, `FOG_LIGHT_CONE_EXTRA` | 3, 4 cells | SP runner radial fog and forward cone extension. |
| `HUNTER_SPAWN_DIST`, `CATCH_DISTANCE_FACTOR` | 18, .6 | Requested BFS spawn distance; pixel catch threshold factor. |
| `PULSE_SPEED`, `PLAYER_GLOW_RADIUS`, `HUNTER_THREAT_RADIUS` | 5, 11, 13 | Animation pulse and geometric glow radii. |
| `TWO_P_HUNTER_RELEASE`, `TWO_P_ROUND_TIME_LIMIT`, `TWO_P_HUNTER_CELLS_PER_SEC` | 3 s, 90 s, 6.6 | 2P hunter release, round limit, AI/human hunter base speed. |
| `TWO_P_RUNNER_LOS_RADIUS`, `TWO_P_HUNTER_LOS_RADIUS` | 3, 6 | 2P runner/hunter visibility radii. |
| `TWO_P_RUNNER_BLIP_CYCLE`, `TWO_P_RUNNER_BLIP_DURATION` | 1.5 s, .25 s | Periodic runner blip in hunter view. |
| `TWO_P_DIVIDER_WIDTH` | 4 px | Split-view divider. |
| `TWO_P_SCORE_ESCAPE_BASE`, `TWO_P_SCORE_ESCAPE_TIME_FACTOR`, `TWO_P_SCORE_SURVIVAL_FACTOR`, `TWO_P_SCORE_CATCH_BASE`, `TWO_P_SCORE_CATCH_TIME_FACTOR`, `TWO_P_SCORE_LIMIT_FACTOR` | 1000, 10, 10, 1000, 10, 10 | Constants used in 2P result formulas. |

### Movement, fog, catch and scoring

World tiles are 32×32 pixels. At base rates, runner moves 7 cells/s and SP hunter 6 cells/s; 2P hunter is set to 6.6 cells/s. The mover interpolates in pixel coordinates between neighboring cell centers, snaps when within one pixel, and only chooses a new turn at a cell center (src/Config.h:72-80; src/GridMover.cpp:32-127). Powerups scale movement speed while their timers are active.

Fog ray-casts from the entity's cell to candidate cells inside a squared-distance circle. It samples each ray at three samples per maximum row/column distance, rounds samples to grid coordinates, blocks diagonal peeking when both orthogonal side cells are walls, and stops on a wall after marking it (src/Player.cpp:48-100; src/Hunter.cpp:80-138). Player also casts three forward rays per cone step out to `radius+forwardExtra`. Discovered cells remain remembered while `visibleNow` is cleared/recomputed each call. Render alpha distinguishes undiscovered (dark), remembered, and current visibility (src/Game.cpp:874-897).

SP hunter spawn uses BFS from `(1,1)` aiming for 18 cells distance; it selects the reachable open candidate closest to that distance (src/Game.cpp:190-202; src/Pathfinder.cpp:94-134). Hunter wake is `max(5, HUNTER_WAKE_TIME - (level-1))` seconds; it starts at 8 seconds and decreases by a second per level, minimum 5 (src/Game.cpp:550-552). In 2P, the runner moves after the 4s movement freeze and the hunter is released after that freeze plus 3s (src/Game.cpp:616-617; src/Config.h:84-87, 107-108).

Catch is checked on logical same cell, swapped cells, or pixel distance below `CATCH_DISTANCE_FACTOR × TILE_SIZE` (19.2 px) in SP and 2P (src/Game.cpp:560-570, 727-739; src/Config.h:96-97). Exit is runner reaching the maze's fixed exit cell. The 2P match has two rounds with roles reversed in round two; it reuses one seed. Round 1 scores are stored per player; round 2 aggregates totals and tie-breaks by escape times (src/Game.cpp:221-245, 607-748, 1035-1089).

SP level completion adds `1000*level + max(0, 600 - 5*seconds)` into run score; a later catch ends the run and submits accumulated run score after name entry (src/Game.cpp:532-538, 560-579). Hunter speed is `min(baseHunter*(1+.05*(level-1)), runnerBase*1.05)`; fog radius is reduced at levels 4 and 7 with minimum 2 (src/Game.cpp:170-219, 523-528). New levels generate another seed.

2P scoring as implemented: escape gives runner `1000 + 10*(90-time)` and hunter zero; caught gives runner `int(10*time)` and hunter `1000 + 10*(90-time)`; timeout gives runner 900 and hunter zero (src/Game.cpp:678-690; constants src/Config.h:117-122). Whether any formula can become negative is limited by the 90-second timer for ordinary outcomes; code path should be consulted if round time is changed.

Powerups are spawned only on open dead-end cells (exactly one open cardinal neighbor), at Manhattan distance at least ten from both current spawns. The candidates are not randomized; the first, one-third, and two-thirds positions in row-major candidate order are selected. One pickup of each type is placed in 2P; only Surge is placed in SP. A pickup respawns at its same location after ten seconds, and only the matching role can take it; each player holds one charge. Surge is runner speed ×1.6 for 2s; Dash is human hunter speed ×1.8 for 1s; Ping reveals the runner for 2s. AI hunter does not activate abilities. P1 activates with Space and P2 with Enter (src/Game.cpp:909-963, 332-335, 403-409; src/Config.h:46-51). If there are few candidates, selected array positions can coincide. Exact retry/round reset behavior is visible in setup methods and should not be confused with role assignment.

Controls are player-bound: P1 always uses WASD, P2 always arrows, including after roles swap. SP handler accepts both input sets for the single runner and uses latest press order; the 2P update routes P1/P2 input objects to player identities, then maps identities to current roles (src/Game.cpp:427-444, 623-645; src/DirectionInput.cpp:18-61).

## 5. RENDERING PIPELINE

`rebuildMazeTexture` draws the static floor and walls once into an `sf::RenderTexture` after generation and exposes the result as a sprite for each camera (src/Game.cpp:104-128). Wall edge highlights are narrow cyan lines only where adjacent cells are floor; floor has a subtle grid. This avoids redrawing each tile as geometry in every frame.

For each player view, `drawPlayerView` applies the camera/view, then draws cached maze, animated exit portal, fog (not during preview), powerups, visible runner/hunter actors, danger/vignette, optional SP BFS overlay, and rect-relative HUD (src/Game.cpp:809-872). Exact checks control role visibility: fog visibility, blinking, 2P periodic runner blip, and active Ping affect whether the opponent appears. `renderFog` constructs a single vertex array of quads for tiles, with alpha selected from undiscovered/remembered/current visibility and a distance falloff for visible cells (src/Game.cpp:874-897). Exit portal is two rotating rings plus pulsing center (src/Game.cpp:899-907).

`buildRadialGradient` precomputes a 128×128 image/texture used as a radial alpha gradient; it is reused for vignette/glow rendering (src/Game.cpp:90-102). Danger red overlay depends on hunter proximity and is disabled by Low FX; white vignette remains (src/Game.cpp:809-872). No custom shaders are present.

SP uses one full 1280×720 rectangle; 2P invokes the same renderer on two half-width rectangles and draws divider/intermission layers (src/Game.cpp:750-807, 965-984). Camera view size is 800×450 SP and 400×450 per half in 2P; the same 32-pixel world tile size is retained. Camera follows target with exponential interpolation and clamps against maze edges (src/Game.cpp:588-603, 661-673; src/Config.h:20-22). HUD shares relative padding/role strip, minimap bottom-right, powerup slot bottom-left; minimap plots discovered cells and relevant entity/exit markers (src/Game.cpp:1184-1314). The 160px minimap and 20px padding are Config values.

F2 toggles Low FX and the HUD shows the mode. Average FPS under 45 for 3 seconds auto-enables it with a toast. Low FX disables danger pulse and (if implemented) particle budget, but this source has no particle system at all; it does not have particle reduction behavior. F4 toggles the FPS readout. Current source also has no screen-shake implementation (src/Game.cpp:260-275, 447-472; src/Game.h fields:110-198).

## 6. IO AND CONFIG

Leaderboard rows are tab-separated: `name<TAB>score<TAB>mode<TAB>detail<TAB>insertionId`, one entry per line. Loading clears tree/ID then parses valid rows and inserts each in file order; malformed rows and integer exceptions are skipped. Names are capped at ten characters. Saving truncates the target and writes BST preorder; adding triggers save immediately (src/Leaderboard.cpp:19-68). On Windows, target path is beside executable using `GetModuleFileNameA`; if that cannot produce a usable path, or on non-Windows, it falls back to relative `leaderboard.txt` (src/Leaderboard.cpp:19-30). Detail text is not escaped/sanitized, so embedded tabs/newlines break the row convention. Save failure is returned but `add` does not report it to UI.

Fonts are loaded by relative paths under `assets/fonts`; fallback candidates include Windows system `segoeui.ttf`, `arial.ttf`, `segoeuib.ttf`, `arialbd.ttf`. Failed font loading logs a message; there is no embedded fallback font (src/Game.cpp:130-157). CMake copies assets next to the executable in the Windows post-build step (CMakeLists.txt:39-52), but relative font lookup still depends on process working directory. The binary font assets are the three listed in Snapshot. No sound/image asset load occurs.

`--seed N` is the sole command-line option; parser uses `stoul`, ignores invalid input via catch, and passes zero/default if not set (src/main.cpp:4-21). In Maze, seed 0 means generate a runtime seed using high-resolution clock XOR `random_device` (src/Maze.cpp:24-33). The 2P match stores one chosen seed and uses it for both rounds (src/Game.cpp:221-245).

## 7. CONTROLS TABLE

Global controls are handled before state-specific dispatch: F1 toggles controls test, F2 Low FX, F4 FPS; window close exits (src/Game.cpp:262-285).

| State/screen | Key | Actual behavior |
|---|---|---|
| Main menu | W/Up, S/Down | Select menu item. |
| Main menu | Enter/Space | Activate selected item; Quit is a menu selection. Esc does not quit main menu. |
| Menu submenu | Esc/Enter | Return to main menu. Leaderboard 1/2/3 changes traversal list. |
| SP preview/playing | WASD or arrows | Move the single runner; most recently pressed held direction wins. |
| SP preview/playing | Space | Activate held powerup; Esc pause; R generate a new maze; F3 BFS overlay. |
| Generating | Space | Finish all remaining generation immediately. Plus/equal increases speed; minus/hyphen decreases it. |
| Paused | F3 | Toggle BFS overlay. MenuList: W/S or Up/Down navigate; Enter/Space resume/retry/menu; Esc resumes. |
| GameOver | MenuList | Retry or main menu (selection via navigation and Enter/Space; Esc is mapped by menu handler). |
| LevelCleared | Enter/Space | Next level. |
| NameEntry | Printable text, max 10 chars | Type name; Backspace deletes; Enter submits. |
| 2P preview/playing | P1 WASD; P2 arrows | Move assigned players regardless of whether runner/hunter role. Esc pauses. |
| 2P preview/playing | P1 Space; P2 Enter | Activate that player's held powerup. |
| 2P paused | MenuList | Resume, restart match on same seed, or menu. |
| 2P intermission | Enter/Space; Esc | Start round two on same seed; or return menu. |
| 2P match over | MenuList | Rematch with new seed or main menu. |
| Controls test overlay | key press | F1/Esc closes; overlay shows key/input state. |

## 8. RISK REGISTER

- **High: repeated level score addition.** `update` does not early-return for `LevelCleared`. It falls through to SP gameplay, and the exit check may add that level's score on each frame spent on the cleared screen. This is a real source-level behavior, not a hypothetical (src/Game.cpp:447-605; state transition in 378-382).
- **High: gameplay continues during name entry.** `NameEntry` also falls through SP update logic, so player timers/movement/powerup and fog may continue while the name prompt is on screen (src/Game.cpp:447-605; text state handling 383-395).
- **Generation visualization metric wording:** it draws at most 12 stack entries and displayed depth is derived from that snapshot, not necessarily total stack size; a deep DFS path can therefore be shown as depth 12 (src/Game.cpp:1453-1476; src/Maze.h:34-38).
- **Leaderboard file robustness:** detail strings can include TSV delimiters; add only cleans name. Save errors are ignored by `add` caller (src/Leaderboard.cpp:64-68).
- **Unbalanced BST:** no balancing means sorted/equivalent-biased insertion sequences can degrade operations and recursive depth to O(n) (src/dsa_lib/BST.h:41-98).
- **Approximate LOS:** sampled rays and rounded sample positions can have edge artifacts; it is not a mathematically exact visibility polygon (src/Player.cpp:48-100).
- **Input focus:** losing focus returns neutral movement but does not reset key history, which could leave a held-key ordering stale until key state is reconciled (src/DirectionInput.cpp:18-23).
- **Movement turns:** turns are only accepted at centers, no buffered turn, and dt remainder is discarded after snapping; this can feel less responsive or slightly frame-rate dependent (src/GridMover.cpp:32-89).
- **Maze dimensions:** oddness is asserted, but minimum supported dimensions are not checked even though generation assumes start `(1,1)` and exit `(rows-2,cols-2)` (src/Maze.cpp:7-22).
- **Potential report-only uncertainty:** exact 8-file historic baseline and exact additions since it are unsure without a provided baseline/history; Git is forbidden by project instructions.
- **Conflict with requested feature expectations:** no particle pool, catch/escape particle bursts or screen shake exist in current source; Low FX therefore cannot reduce particle count (src/Game.h:110-198; no particle state; src/Game.cpp rendering/update paths).
- **Potential UI overlap:** leaderboard traversal/tree help text and bottom return hint share fixed screen coordinates; whether they overlap depends on tree/list height (src/Game.cpp:1332-1373).

## 9. VIVA PREP

### Fifteen likely questions and grounded answers

1. **How does iterative DFS make the maze?** `Maze::stepGenerate` looks at the stack top, opens a random unvisited neighbor two grid cells away plus the wall between, then pushes it; if none exists it pops. This is the iterative version of recursive backtracking (src/Maze.cpp:55-114).
2. **Why use a stack there?** DFS needs to return to the most recently visited branch when it reaches a dead end, which is exactly LIFO behavior. The custom `Stack<pair<int,int>>` stores that current route (src/Maze.h:28-38; src/dsa_lib/Stack.h:68-79).
3. **How is braid generation different?** It scans remaining walls for candidate connectors, shuffles them, then removes a fraction to make loops/fewer dead ends. The scan/shuffle occurs at transition and each selected wall removal is stepped separately (src/Maze.cpp:63-80, 116-129).
4. **Why use BFS for the hunter?** Every maze edge has equal cost, so breadth-first search finds a path with the fewest cell moves. The hunter follows only the first step and searches again at its next cell (src/Pathfinder.cpp:11-91; src/Hunter.cpp:14-45).
5. **How does custom Queue implement FIFO?** It keeps head and tail pointers: enqueue links after tail and dequeue removes head; both are constant time. Pathfinder uses it as the frontier (src/dsa_lib/Queue.h:66-84; src/Pathfinder.cpp:25-68).
6. **What is stored in a BST node?** One value plus left and right raw pointers. The tree also tracks root, node count and comparator (src/dsa_lib/BST.h:12-24).
7. **How does BST handle duplicate keys?** If the comparator says neither value is less than the other, insert assigns the new value to that existing node rather than making another node. For leaderboard, the insertion ID normally distinguishes equal score/name rows (src/dsa_lib/BST.h:41-55; src/Leaderboard.cpp:9-13).
8. **Explain BST removal with two children.** It chooses the smallest value in the right subtree, copies it into the node being removed, then recursively removes that successor. The successor has no left child, so its deletion reduces to a simpler case (src/dsa_lib/BST.h:82-96).
9. **Why save the leaderboard in preorder?** Re-inserting root-left-right order recreates the same BST shape when comparator and values are unchanged. That matters because this tree is unbalanced and insertion order affects both its shape and operation cost (src/Leaderboard.cpp:54-61; src/dsa_lib/BST.h:116-122).
10. **What is the BST's worst case?** If values repeatedly arrive in order that always extends one side, height becomes n, so search/insert/remove become O(n) and recursive functions use O(n) call stack. This implementation has no balancing rotations (src/dsa_lib/BST.h:41-98, 100-106).
11. **How does mover collision with walls work?** At a cell center it converts the requested direction into a neighbor cell and asks `Maze::isWall`; movement starts only if that neighbor is open. During a segment it interpolates pixel position and snaps at the target (src/GridMover.cpp:32-89).
12. **How is fog remembered?** Each player has a persistent `discovered_` grid and a temporary `visibleNow_` grid. Every visibility update clears only the temporary map, casts rays and sets both for cells seen now (src/Player.cpp:23-48).
13. **What determines the leaderboard ordering?** The comparator orders scores and then names/IDs in the reverse sense needed for reverse-inorder, so reverse-inorder produces highest score first and alphabetical names for ties (src/Leaderboard.cpp:9-13; src/Leaderboard.cpp:71-77).
14. **What exactly runs each frame?** `run` clamps dt, polls events, updates, then renders. Update has early-return state gates before it reaches the active SP or 2P gameplay code (src/Game.cpp:247-258, 447-605).
15. **How are SP and split-screen rendering kept consistent?** Both call `drawPlayerView` with the same role/entity/fog inputs; SP passes the full window rectangle and 2P passes two half rectangles. The shared function draws cached maze, effects, actors and rect-relative HUD (src/Game.cpp:750-872, 965-984).

### Weak spots a professor could probe

- **“Why does score change while the cleared screen is visible?”** The update function lacks a `LevelCleared` early return, so the SP exit branch can add it repeatedly. That is a bug in current code, not intended scoring.
- **“Why does the player keep moving in the name prompt?”** NameEntry also lacks an early return in the SP update path; gameplay update continues. That should be fixed separately, but this report task made no source changes.
- **“Is this BST always fast?”** No. It is unbalanced; worst-case height and operations are linear, and recursion can become deep.
- **“Does `stepGenerate` always perform one operation?”** No. Its DFS/BRAID removal steps are incremental, but transition to braid performs a whole-grid scan and shuffle in one call.
- **“Is LOS exact?”** No. It uses sampled/rounded rays, a practical grid approximation.
- **“Can leaderboard detail contain tabs?”** It can be passed in, but the writer does not escape it; tabs/newlines can shift or split columns/rows.
- **“Why are there no particle effects in Low FX?”** Current source has no particle implementation or shake state. Low FX disables the danger pulse and has FPS/toast UI, but cannot reduce absent particles.
- **“Which files were added after the original eight?”** Exact list is unsure: the baseline was not provided and Git/history inspection is prohibited by the repository rules.
- **“Why doesn’t the main menu Esc quit?”** Event handling returns from submenus on Esc, but main menu action is driven by menu selection; close-window is the reliable quit action.

## 10. GLOSSARY

| Term | Meaning in this project |
|---|---|
| BFS | Breadth-first search; visits reachable cells in increasing move distance using a FIFO queue. |
| DFS | Depth-first search; maze carving follows one branch until dead end and backtracks. |
| BST | Binary search tree; leaderboard storage and traversal structure. |
| LIFO / FIFO | Last-in-first-out stack order / first-in-first-out queue order. |
| Cell / tile | A row-column maze location; rendered as a 32×32-pixel square. |
| Seed | Integer initializing maze random-number generation; same seed aims to reproduce layout. |
| Braid / braiding | Removing selected remaining walls to add loops and reduce dead ends. |
| `visitOrder` | BFS queue-removal order, used for animated demo wave. |
| Parent / predecessor | Previous BFS cell used to reconstruct the shortest path. |
| Path | Ordered grid cells from BFS start to goal. |
| LOS | Line of sight; sampled grid rays determine currently visible cells. |
| `discovered` | Cells seen at any earlier time, retained for fog-of-war memory. |
| `visibleNow` | Cells visible on the current visibility update only. |
| View / viewport | SFML camera world region and the screen rectangle it draws into. |
| Pre-render / cached maze | Static maze drawn once into a texture and reused by views. |
| Vignette | Radial dark/red overlay near view edges or danger state. |
| Ping / blip | Temporary opponent location reveal / periodic runner indicator in 2P hunter view. |
| Low FX | Reduced-effects toggle; disables danger pulse, with auto-enable on sustained low average FPS. |
| P1 / P2 | Player identities; P1 uses WASD and P2 arrow keys regardless of current role. |
| Role | Runner or hunter assignment; in 2P the assignments swap for round two. |
| Charge | One held, unactivated power-up on a player. |
| `maxQueueSize` | Largest BFS frontier size observed during one search. |
| `height` | Number of nodes on the longest root-to-leaf BST path; empty tree is zero. |
| Preorder / inorder / postorder | BST traversal orders: node-left-right / left-node-right / left-right-node. |
| Insertion ID | Monotonically assigned leaderboard tie-break field. |
| `dt` | Elapsed seconds since previous rendered frame, clamped to the configured maximum. |
| SFML | Simple and Fast Multimedia Library; this project uses its graphics, window, and system modules. |
