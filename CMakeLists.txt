cmake_minimum_required(VERSION 3.31.1)
project(gptvsdeepseek VERSION 0.1.0 LANGUAGES C CXX)

list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/vcpkg_installed/x64-windows")

find_package(SFML COMPONENTS system window graphics audio CONFIG REQUIRED)

add_executable(gptvsdeepseek main.cpp)


target_link_libraries(gptvsdeepseek PRIVATE sfml-system sfml-network sfml-graphics sfml-window sfml-audio)
target_link_libraries(gptvsdeepseek PRIVATE sfml-main)