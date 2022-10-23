add_definitions(-std=c++2a)

file(GLOB_RECURSE TARGETS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_library(hnll_physics STATIC ${TARGETS})

target_include_directories(hnll_physics PUBLIC
    $ENV{HNLL_ENGN}/include
    $ENV{HNLL_ENGN}/submodules/imgui
    $ENV{GLM_DIR}
)