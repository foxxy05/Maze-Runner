# Maze Chase - agent rules
C++17, SFML 2.6.1, college Data Structures project. Phases 1-3 are done (fixes, menu/HUD/camera, 2-player split screen).
- dsa_lib (Stack, Queue, BST) must stay custom. Do not use std::stack/queue/map for DSA-visible parts.
- Target: laptop with a simple integrated GPU, 60 FPS. No custom GLSL shaders, no bloom, no audio, no networking.
- Only touch files needed. Do not rename or restructure working code. Adapt to existing names.
- All tunables go in src/Config.h (namespace cfg, constexpr). Keep "DSA CORE" comment banners on algorithm code.
- Controls are bound to the player: P1 = WASD, P2 = arrows, always, roles swap but keys never do.
- Do not run git commands. Do not run a full SFML dependency build if it is slow or fails in the sandbox; stop and tell me instead.
- Do not read the whole repo. Read only the files named in the task.
- Finish with: files changed, how to test, known gaps. Keep it short.