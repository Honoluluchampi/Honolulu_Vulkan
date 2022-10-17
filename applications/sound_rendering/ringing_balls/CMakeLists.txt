cmake_minimum_required(VERSION 3.16)

project(ringing_balls)

# specify the c++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "-g3 -O0")

# build engine
add_subdirectory($ENV{HNLL_ENGN}/ $ENV{HNLL_ENGN}/build)
set(SOURCES ringing_balls.cpp)
add_executable(ringing_balls ${SOURCES})
target_include_directories(ringing_balls PUBLIC $ENV{HNLL_ENGN}/include include)
target_link_libraries(ringing_balls PUBLIC hnll_engine)
