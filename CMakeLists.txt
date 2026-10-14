cmake_minimum_required(VERSION 3.16)
project(MazeChase LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- SFML ---
set(SFML_BUILD_AUDIO OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_DOC OFF CACHE BOOL "" FORCE)

include(FetchContent)
FetchContent_Declare(
    SFML
    URL https://github.com/SFML/SFML/archive/refs/tags/2.6.1.zip
)
FetchContent_MakeAvailable(SFML)

add_executable(MazeChase
    src/main.cpp
    src/Maze.cpp
    src/Player.cpp
    src/Hunter.cpp
    src/DirectionInput.cpp
    src/Pathfinder.cpp
    src/Game.cpp
    src/UI.cpp
    src/GridMover.cpp
)

target_include_directories(MazeChase PRIVATE src)
target_compile_options(MazeChase PRIVATE -Wall -Wextra)

target_link_libraries(MazeChase PRIVATE
    sfml-graphics
    sfml-window
    sfml-system
)

if(WIN32)
    add_custom_command(
        TARGET MazeChase POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:sfml-graphics> $<TARGET_FILE:sfml-window> $<TARGET_FILE:sfml-system>
            $<TARGET_FILE_DIR:MazeChase>
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/assets
            $<TARGET_FILE_DIR:MazeChase>/assets
    )
endif()
