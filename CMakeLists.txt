cmake_minimum_required(VERSION 3.16...3.32)
project(luna LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(SFML 3 COMPONENTS Graphics Window System REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(TBB REQUIRED)
find_package(reflectcpp CONFIG REQUIRED)
find_package(OpenMP REQUIRED)

set(LUNA_INCLUDE_DIRS
    include
    include/gfx
    include/physics
    include/solvers
    include/integrators
    include/cfg
)

# Sources shared between both executables
set(LUNA_CORE_SOURCES
    src/luna_engine.cpp
    src/cfg/sim_config.cpp
    src/cfg/rand_config.cpp
    src/solvers/barnes_hut_openmp.cpp
    src/solvers/barnes_hut_single_threaded.cpp
    src/solvers/barnes_hut_tbb.cpp
    src/solvers/direct_openmp.cpp
    src/solvers/direct_single_threaded.cpp
    src/solvers/direct_tbb.cpp
    src/integrators/leapfrog_openmp.cpp
    src/integrators/leapfrog_tbb.cpp
    src/physics/octree.cpp
    src/physics/ephemeris.cpp
)

# GUI-only sources
set(LUNA_GUI_SOURCES
    src/gfx/renderer.cpp
    src/gfx/camera.cpp
)

# Compile all shared sources once into an object library consumed by both targets
add_library(luna_core OBJECT ${LUNA_CORE_SOURCES})
target_include_directories(luna_core PUBLIC ${LUNA_INCLUDE_DIRS})
target_link_libraries(luna_core PUBLIC
    TBB::tbb
    OpenMP::OpenMP_CXX
    reflectcpp::reflectcpp
)
target_compile_options(luna_core PUBLIC
    -O3
    -march=native
    -ffast-math
    -flto
    -fno-signed-zeros
    -fomit-frame-pointer
)

# GUI executable
add_executable(luna_gui src/main.cpp ${LUNA_GUI_SOURCES})
target_compile_definitions(luna_gui PRIVATE LUNA_GUI)
target_link_libraries(luna_gui PRIVATE
    luna_core
    SFML::Graphics
    SFML::Window
    SFML::System
    OpenGL::GL
    glad::glad
    glfw
    glm::glm
)

# Headless executable (no windowing, no OpenGL)
add_executable(luna_headless src/main.cpp)
target_link_libraries(luna_headless PRIVATE luna_core)

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
