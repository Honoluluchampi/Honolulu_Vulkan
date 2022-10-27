# c++20 is available in VERSION 3.16 or later
cmake_minimum_required(VERSION 3.16)

# set the project name and its version
project(app VERSION 1.0)
project(hnll_engine VERSION 1.0)

# this header file passes the version number to the source code
# configure_file(testConfig.h.in testConfig.h)

# specify the c++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "-g3 -O0")

# build engine
file(GLOB_RECURSE SOURCES modules/game/*.cpp)
add_library(hnll_engine STATIC ${SOURCES})
add_subdirectory(modules/audio)
add_subdirectory(modules/geometry)
add_subdirectory(modules/graphics)
add_subdirectory(modules/physics)
add_subdirectory(modules/gui)
add_subdirectory(modules/utils)

target_include_directories(hnll_engine PUBLIC
        include/
        submodules/imgui
        submodules/tiny_obj_loader
        submodules/glfw
        submodules/glm)

target_link_directories(hnll_engine PUBLIC $ENV{VULKAN_DIR}/lib)

target_link_libraries(hnll_engine PUBLIC
        hnll_audio
        hnll_physics
        hnll_gui
        hnll_graphics
        hnll_geometry
        hnll_utils
        vulkan
        glfw
        )

# build the example
add_executable(app main.cpp)
target_link_libraries(app PUBLIC hnll_engine)