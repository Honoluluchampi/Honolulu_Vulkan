cmake_minimum_required(VERSION 3.16)

project(view_frustum_culling)

# specify the c++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "-g3 -O0")

# build engine
add_subdirectory($ENV{HNLL_ENGN}/ $ENV{HNLL_ENGN}/build)
set(SOURCES view_frustum_culling.cpp)
add_executable(view_frustum_culling ${SOURCES})
target_include_directories(view_frustum_culling PUBLIC $ENV{HNLL_ENGN}/include include)
target_link_libraries(view_frustum_culling PUBLIC hnll_engine)
