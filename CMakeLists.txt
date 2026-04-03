cmake_minimum_required(VERSION 3.3)
project(luna_cpp)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(include)
include_directories(include/gfx)

add_executable(luna_cpp
        include/constants.hpp
        include/sim_state.hpp
        src/main.cpp
        src/sim_state.cpp)
