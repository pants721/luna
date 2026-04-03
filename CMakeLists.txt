cmake_minimum_required(VERSION 3.16...3.25)
project(luna LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# SFML
find_package(SFML 3 COMPONENTS Graphics Window System REQUIRED)
find_package(OpenGL REQUIRED)
find_package(TBB REQUIRED)

include_directories(include)
include_directories(include/gfx)

add_executable(luna
        src/main.cpp
        src/ephemeris.cpp)

target_link_libraries(luna 
    PRIVATE 
    SFML::Graphics
    SFML::Window
    SFML::System
    OpenGL::GL
)
target_link_libraries(luna PRIVATE TBB::tbb)

target_compile_options(luna PRIVATE -ltbb)

find_program(CPPCHECK_EXE NAMES cppcheck)

if(CPPCHECK_EXE)
    set(CMAKE_CXX_CPPCHECK 
        ${CPPCHECK_EXE}
        "--enable=all"
        "--inconclusive"
        "--force" 
        "--inline-suppr"
        "--std=c++20"
        "--suppress=missingIncludeSystem" # Hides noise from SFML/System headers
        "--addon=misra"
    )
    message(STATUS "Cppcheck found: ${CPPCHECK_EXE}")
else()
    message(WARNING "Cppcheck NOT found! Static analysis will be skipped.")
endif()
