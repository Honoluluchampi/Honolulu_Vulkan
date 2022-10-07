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
file(GLOB_RECURSE SOURCES modules/*.cpp)
add_library(hnll_engine STATIC ${SOURCES})

target_include_directories(hnll_engine PUBLIC include/ submodules/imgui submodules/tiny_obj_loader)
target_link_directories(hnll_engine PUBLIC $ENV{VULKAN_DIR}/lib)
if (APPLE)
    target_link_directories(hnll_engine PUBLIC $ENV{VULKAN_DIR}/lib $ENV{GLFW_DIR}/lib)
endif (APPLE)

target_link_libraries(hnll_engine PUBLIC vulkan glfw)

# build the example
add_executable(app main.cpp)
target_link_libraries(app PUBLIC hnll_engine)

# build the test
