cmake_minimum_required(VERSION 3.16)

project(falling_ball)

# specify the c++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "-g3 -O0")

# build engine
add_subdirectory($ENV{HNLL_ENGN}/ $ENV{HNLL_ENGN}/build)
set(SOURCES src/falling_ball.cpp)
add_executable(falling_ball ${SOURCES})
target_include_directories(falling_ball PUBLIC $ENV{HNLL_ENGN}/include include)
target_link_libraries(falling_ball PUBLIC hnll_engine)
