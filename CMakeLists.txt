cmake_minimum_required(VERSION 3.13)
project(bfc C CXX)
set(CMAKE_C_STANDARD 11)

# Emit a compile_commands.json for use with clangd in LSPs
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

add_subdirectory(lib/bobtail)
add_executable(bfc
    main.c
)
target_link_libraries(bfc PRIVATE bobtail)
