cmake_minimum_required(VERSION 3.16)
project(DeepDark)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Collect all source files recursively
file(GLOB_RECURSE SOURCES 
    "${CMAKE_SOURCE_DIR}/Deep_Dark/source/*.cpp"
    "${CMAKE_SOURCE_DIR}/Deep_Dark/Deep_Dark.cpp"
)

set(SFML_DIR "/home/deck/Desktop/vs-code-projects/SFML-3.1.0/lib/cmake/SFML")

find_package(SFML 3 REQUIRED COMPONENTS Graphics Window System)

add_executable(DeepDark ${SOURCES})

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

target_precompile_headers(DeepDark PRIVATE /home/deck/Desktop/vs-code-projects/Deep-Dark/Deep_Dark/include/utilities/pch.h)

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
    # add subdirs here or just the root if headers include each other relatively
)

target_link_libraries(DeepDark PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::System
)  