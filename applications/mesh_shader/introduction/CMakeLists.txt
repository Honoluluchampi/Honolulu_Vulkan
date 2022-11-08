# MESH SHADER SAMPLE
cmake_minimum_required(VERSION 3.16)
project(introduction VERSION 1.0)

# specify the c++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "-g3 -O0")

add_subdirectory($ENV{HNLL_ENGN}/ $ENV{HNLL_ENGN}/build)
set(SOURCES introduction.cpp)
add_executable(introduction ${SOURCES})
target_include_directories(introduction PUBLIC
        $ENV{HNLL_ENGN}/include
        include
        $ENV{HNLL_ENGN}/submodules/extensions)
target_link_libraries(introduction PUBLIC hnll_engine)