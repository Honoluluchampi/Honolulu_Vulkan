cmake_minimum_required(VERSION 3.16)
project(animation_meshlet VERSION 1.0)

# specify the c++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "-g3 -O0")

add_subdirectory($ENV{HNLL_ENGN}/ $ENV{HNLL_ENGN}/build)
set(SOURCES animation_meshlet.cpp)
add_executable(animation_meshlet ${SOURCES})
target_include_directories(animation_meshlet PUBLIC
        $ENV{HNLL_ENGN}/include
        include
        $ENV{HNLL_ENGN}/submodules)
target_link_libraries(animation_meshlet PUBLIC hnll_engine)