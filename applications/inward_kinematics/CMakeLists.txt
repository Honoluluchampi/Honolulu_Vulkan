cmake_minimum_required(VERSION 3.16)
project(inward_kinematics VERSION 1.0)

# specify the c++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "-g3 -O0")

# build engine
add_subdirectory($ENV{HNLL_ENGN}/ $ENV{HNLL_ENGN}/build)

# build inward-kinematics
#file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
set(SOURCES src/main.cpp src/app.cpp src/control_point.cpp src/drag_manager.cpp)
add_executable(inward_kinematics ${SOURCES})
target_include_directories(inward_kinematics PUBLIC $ENV{HNLL_ENGN}/include include)
target_link_libraries(inward_kinematics PUBLIC hnll_engine)