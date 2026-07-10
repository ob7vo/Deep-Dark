cmake_minimum_required(VERSION 3.20)
project(DeepDark)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

# ---------------------------------------------------------------------------
# SFML 3.1.0
# ---------------------------------------------------------------------------
set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.1.0
    GIT_SHALLOW ON
)
FetchContent_MakeAvailable(SFML)

# ---------------------------------------------------------------------------
# Dear ImGui — pin to a tag known compatible with imgui-sfml below
# ---------------------------------------------------------------------------
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.1
    GIT_SHALLOW ON
)
FetchContent_MakeAvailable(imgui)

# ---------------------------------------------------------------------------
# ImGui-SFML — 3.0 targets SFML 3.0.0 + ImGui 1.91.1
# ---------------------------------------------------------------------------
set(IMGUI_DIR ${imgui_SOURCE_DIR})
set(IMGUI_SFML_FIND_SFML OFF)   # SFML already provided above via FetchContent

FetchContent_Declare(
    imgui-sfml
    GIT_REPOSITORY https://github.com/SFML/imgui-sfml.git
    GIT_TAG v3.0
    GIT_SHALLOW ON
)
FetchContent_MakeAvailable(imgui-sfml)

# Collect all source files recursively
file(GLOB_RECURSE SOURCES 
    "${CMAKE_SOURCE_DIR}/Deep_Dark/source/*.cpp"
    "${CMAKE_SOURCE_DIR}/Deep_Dark/Deep_Dark.cpp"
)

#*
#set(IMGUI_DIR "/home/deck/Libraries/imgui")          # <-- adjust to your actual path
#set(IMGUI_SFML_DIR "/home/deck/Libraries/imgui-sfml")
#set(SFML_DIR "/home/deck/Libraries/SFML-3.1.0/lib/cmake/SFML")
#

add_executable(DeepDark ${SOURCES})

target_precompile_headers(DeepDark PRIVATE ${CMAKE_SOURCE_DIR}/Deep_Dark/include/utilities/pch.h)

target_include_directories(DeepDark PRIVATE
    ${CMAKE_SOURCE_DIR}/Deep_Dark/include
    ${CMAKE_SOURCE_DIR}/Deep_Dark/include/utilities
    ${CMAKE_SOURCE_DIR}/Deep_Dark/include/utilities/main
    ${CMAKE_SOURCE_DIR}/Deep_Dark/include/configs
    ${CMAKE_SOURCE_DIR}/Deep_Dark/include/enums
    ${CMAKE_SOURCE_DIR}/Deep_Dark/include/game_states
    ${CMAKE_SOURCE_DIR}/Deep_Dark/include/menus
    ${CMAKE_SOURCE_DIR}/Deep_Dark/include/saves
    ${CMAKE_SOURCE_DIR}/Deep_Dark/include/stage/entities
    ${CMAKE_SOURCE_DIR}/Deep_Dark/include/stage/main
    ${CMAKE_SOURCE_DIR}/Deep_Dark/include/stage/mechanics
    ${CMAKE_SOURCE_DIR}/Deep_Dark/include/stage/unit
    ${CMAKE_SOURCE_DIR}/Deep_Dark/include/systems
    ${CMAKE_SOURCE_DIR}/Deep_Dark/include/systems/managers
    ${CMAKE_SOURCE_DIR}/Deep_Dark/include/systems/observers
    ${CMAKE_SOURCE_DIR}/Deep_Dark/include/visuals
    ${CMAKE_SOURCE_DIR}/Deep_Dark/include/visuals/camera
    ${CMAKE_SOURCE_DIR}/Deep_Dark/include/visuals/textures
    ${CMAKE_SOURCE_DIR}/Deep_Dark/include/visuals/ui
    ${IMGUI_DIR}
    ${IMGUI_SFML_DIR}
)

target_link_libraries(DeepDark PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::System
    ImGui-SFML::ImGui-SFML
)

target_compile_definitions(DeepDark PRIVATE
    IMGUI_USER_CONFIG="${IMGUI_SFML_DIR}/imconfig-SFML.h"
)

add_custom_command(TARGET DeepDark POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/Deep_Dark/fonts
    $<TARGET_FILE_DIR:DeepDark>/fonts
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/Deep_Dark/sprites
    $<TARGET_FILE_DIR:DeepDark>/sprites
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/Deep_Dark/game_saves
    $<TARGET_FILE_DIR:DeepDark>/game_saves
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/Deep_Dark/configs
    $<TARGET_FILE_DIR:DeepDark>/configs
)
