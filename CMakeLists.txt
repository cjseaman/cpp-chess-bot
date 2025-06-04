cmake_minimum_required(VERSION 3.10.0)
project(cpp-chess-bot VERSION 0.1.0 LANGUAGES C CXX)
include(FetchContent)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
link_libraries(ws2_32 wsock32)
add_executable(cpp-chess-bot chess_main.cpp src/chess.cpp src/tests.cpp src/networking.cpp include/chess.hpp)

