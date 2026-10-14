#pragma once

namespace cfg {

// Window & Display
constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;
constexpr int TARGET_FPS = 60;

// Grid & Tile Dimensions
constexpr int MAZE_ROWS = 39;
constexpr int MAZE_COLS = 39;
constexpr int TILE_SIZE = 32;

static_assert(MAZE_ROWS % 2 != 0, "MAZE_ROWS must be odd");
static_assert(MAZE_COLS % 2 != 0, "MAZE_COLS must be odd");

// Camera Controls (sf::View showing ~25x14 tiles)
constexpr float CAMERA_VIEW_W = 800.0f;
constexpr float CAMERA_VIEW_H = 450.0f;
constexpr float CAMERA_LERP_SPEED = 7.0f;

// Minimap Dimensions
constexpr float MINIMAP_SIZE = 160.0f;
constexpr float MINIMAP_PADDING = 20.0f;

// Menu Background Animation
constexpr float MENU_BG_GRID_SPACING = 40.0f;
constexpr float MENU_BG_SPEED = 12.0f;

// Maze Generation Constants
constexpr int WALL_TILE = 1;
constexpr int PATH_TILE = 0;
constexpr int STEP_DIST = 2;
constexpr float BRAID_PERCENT = 0.18f;
constexpr int NUM_DIRECTIONS = 4;
constexpr float GENERATION_DFS_SECONDS = 4.0f;
constexpr float GENERATION_BRAID_SECONDS = 0.8f;
constexpr int GENERATION_STACK_VISIBLE = 12;
constexpr float BFS_DEMO_WAVE_SECONDS = 1.2f;
// Rendering / accessibility
constexpr float LOW_FX_FPS_THRESHOLD = 45.0f;
constexpr float LOW_FX_TRIGGER_SECONDS = 3.0f;
constexpr int LOW_FX_PARTICLE_CAP = 40;
constexpr int PARTICLE_CAP = 150;
constexpr float DANGER_DISTANCE_TILES = 11.0f;
constexpr float PORTAL_ROTATION_SPEED = 70.0f;
constexpr float POWERUP_RESPAWN_SECONDS = 10.0f;
constexpr float SURGE_DURATION = 2.0f;
constexpr float DASH_DURATION = 1.0f;
constexpr float SURGE_MULTIPLIER = 1.6f;
constexpr float DASH_MULTIPLIER = 1.8f;
constexpr float PING_DURATION = 2.0f;
constexpr int GENERATION_DFS_STEPS_PER_FRAME = 6;
constexpr int GENERATION_BRAID_STEPS_PER_FRAME = 1;

// Speeds (Cells per second)
constexpr float PLAYER_CELLS_PER_SEC = 7.0f;
constexpr float HUNTER_CELLS_PER_SEC = 6.0f;

// Movement & Animation Controls
constexpr float SNAP_THRESHOLD = 1.0f;
constexpr float HUNTER_SNAP_THRESHOLD = 1.0f;
constexpr float ANIM_FRAME_TIME = 0.10f;
constexpr int TOTAL_ANIM_FRAMES = 4;
constexpr float HUNTER_ANIM_FRAME_TIME = 0.10f;
constexpr int HUNTER_TOTAL_FRAMES = 4;

// Timers & Delays
constexpr float MAP_PREVIEW_DURATION = 5.0f;
constexpr float MOVEMENT_FREEZE_TIME = 4.0f;
constexpr float HUNTER_WAKE_TIME = 8.0f;
constexpr float DELTA_TIME_CAP = 0.1f;

// Radar & Fog Controls
constexpr float HUNTER_BLINK_CYCLE = 1.5f;
constexpr float HUNTER_BLINK_ON_TIME = 0.25f;
constexpr int FOG_VISION_RADIUS = 3;
constexpr int FOG_LIGHT_CONE_EXTRA = 4;

// Hunter Mechanics
constexpr int HUNTER_SPAWN_DIST = 18;
constexpr float CATCH_DISTANCE_FACTOR = 0.6f;

// Display & Diagnostics
constexpr float PULSE_SPEED = 5.0f;
constexpr bool DEBUG_LOG = true;

// Render Constants (RenderUtils)
constexpr int MAX_COUNTDOWN_DIGIT = 5;
constexpr int MIN_COUNTDOWN_DIGIT = 1;
constexpr float PLAYER_GLOW_RADIUS = 11.0f;
constexpr float HUNTER_THREAT_RADIUS = 13.0f;
constexpr float DIGIT_WIDTH = 24.0f;
constexpr float DIGIT_HEIGHT = 38.0f;
constexpr float DIGIT_THICKNESS = 5.0f;

// Phase 3: Local 2-Player Split Screen Constants
constexpr float TWO_P_HUNTER_RELEASE = 3.0f;
constexpr float TWO_P_ROUND_TIME_LIMIT = 90.0f;
constexpr float TWO_P_HUNTER_CELLS_PER_SEC = 6.6f;
constexpr int TWO_P_RUNNER_LOS_RADIUS = 3;
constexpr int TWO_P_HUNTER_LOS_RADIUS = 6;
constexpr float TWO_P_RUNNER_BLIP_CYCLE = 1.5f;
constexpr float TWO_P_RUNNER_BLIP_DURATION = 0.25f;
constexpr float TWO_P_DIVIDER_WIDTH = 4.0f;

// Phase 3 Scoring
constexpr int TWO_P_SCORE_ESCAPE_BASE = 1000;
constexpr int TWO_P_SCORE_ESCAPE_TIME_FACTOR = 10;
constexpr int TWO_P_SCORE_SURVIVAL_FACTOR = 10;
constexpr int TWO_P_SCORE_CATCH_BASE = 1000;
constexpr int TWO_P_SCORE_CATCH_TIME_FACTOR = 10;
constexpr int TWO_P_SCORE_LIMIT_FACTOR = 10;

} // namespace cfg
