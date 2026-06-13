cmake_minimum_required(VERSION 3.10)
project(PlateOfFate)

set(CMAKE_CXX_STANDARD 17)

# UTF-8 for MSVC
if(MSVC)
    add_compile_options(/utf-8)
endif()

add_executable(PlateOfFate
    src/main.cpp
    src/csv_loader.cpp
    src/display.cpp
    src/game_logic.cpp
)

target_include_directories(PlateOfFate PRIVATE src)

# Copy data folder next to executable
add_custom_command(TARGET PlateOfFate POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/data $<TARGET_FILE_DIR:PlateOfFate>/data
)
