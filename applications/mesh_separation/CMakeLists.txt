cmake_minimum_required(VERSION 3.16)

project(mesh_separation)

# specify the c++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "-g3 -O0")

# build engine
add_subdirectory($ENV{HNLL_ENGN}/ $ENV{HNLL_ENGN}/build)
set(SOURCES src/mesh_separation.cpp)
add_executable(mesh_separation ${SOURCES})
target_include_directories(mesh_separation PUBLIC $ENV{HNLL_ENGN}/include include)
target_link_libraries(mesh_separation PUBLIC hnll_engine)
