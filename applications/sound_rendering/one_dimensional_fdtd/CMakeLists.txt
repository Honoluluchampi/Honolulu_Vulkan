cmake_minimum_required(VERSION 3.16)

project(one_dimensional_fdtd)

# specify the c++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "-g3 -O0")

# build engine
add_subdirectory($ENV{HNLL_ENGN}/ $ENV{HNLL_ENGN}/build)
set(SOURCES one_dimensional_fdtd.cpp)
add_executable(one_dimensional_fdtd ${SOURCES})
target_include_directories(one_dimensional_fdtd PUBLIC $ENV{HNLL_ENGN}/include include)
target_link_libraries(one_dimensional_fdtd PUBLIC hnll_engine)
