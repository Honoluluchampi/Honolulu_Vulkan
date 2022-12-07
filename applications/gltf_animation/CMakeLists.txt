cmake_minimum_required(VERSION 3.16)
project(gltf_animation VERSION 1.0)

# specify the c++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "-g3 -O0")

add_subdirectory($ENV{HNLL_ENGN}/ $ENV{HNLL_ENGN}/build)
set(SOURCES gltf_animation.cpp)
add_executable(gltf_animation ${SOURCES})
target_include_directories(gltf_animation PUBLIC
        $ENV{HNLL_ENGN}/include
        include
        $ENV{HNLL_ENGN}/submodules)
target_link_libraries(gltf_animation PUBLIC hnll_engine)