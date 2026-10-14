# Maze Chase — Phase 1

Real-time chase game: navigate a fogged maze while a hunter live-recomputes
BFS pathfinding to hunt you down. Reach the green exit tile to win.

## What's implemented (Phase 1 scope)
- Maze generation ported from your original C stack-based backtracker (`Maze.cpp`)
- Player movement + fog of war (`Player.cpp`) — you only see cells you've explored
- Hunter AI: recomputes shortest path (BFS) to you every ~0.35s and steps toward
  you (`Hunter.cpp`) — this is your graph-traversal algorithm running live as
  an enemy, not a static demo
- Win/lose state, press `R` to regenerate maze and retry

## Not yet built (later phases — see the roadmap discussed in chat)
- Leaderboard (BST) + sort algorithm comparison (bubble/insertion/merge)
- Power-ups, multiple levels, difficulty scaling
- ImGui HUD/menu
- Your original `dsa_lib` isn't wired in yet — I used `std::stack`/`std::queue`
  here to get a runnable skeleton fast. Swapping these for your custom
  templated Stack/Queue is a good next step and keeps the "custom DSA lib"
  requirement satisfied — happy to do that pass next.

## Build (Windows, easiest path — vcpkg)
```
git clone https://github.com/microsoft/vcpkg
.\vcpkg\bootstrap-vcpkg.bat
.\vcpkg\vcpkg install sfml
cmake -B build -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build
```

## Build (any platform — CMake FetchContent, no vcpkg needed)
The included `CMakeLists.txt` already fetches SFML 2.6.1 from GitHub
automatically on first configure. Just run:
```
cmake -B build
cmake --build build
```
First build will take a while (compiling SFML from source). Subsequent
builds are fast.

## Controls
Arrow keys to move. `R` to restart. Close window to quit.

## A note on this handoff
I wrote and reviewed this code but haven't compiled/run it myself — this
sandbox has no display server, so an SFML window can't actually open here.
Build it locally and ping me with any compiler errors; SFML API mismatches
between minor versions are the most likely snag.
