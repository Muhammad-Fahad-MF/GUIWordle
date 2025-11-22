cmake_minimum_required(VERSION 3.25)
project(WordleSDL2 C)

set(CMAKE_C_STANDARD 99)

# Compiler Flags
if(MSVC)
    add_compile_options(/W4 /O2)
else()
    add_compile_options(-Wall -Wextra -O2)
endif()

# Find Packages
find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2_ttf CONFIG REQUIRED)

# --- Link all source files here ---
add_executable(${PROJECT_NAME}
        main.c
        game_logic.c
        renderer.c
        input.c
        icon.rc
)

target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2 SDL2::SDL2main SDL2_ttf::SDL2_ttf)

if(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE m)
endif()

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/words.txt DESTINATION ${CMAKE_CURRENT_BINARY_DIR})