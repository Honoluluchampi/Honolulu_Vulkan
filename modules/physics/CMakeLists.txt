add_definitions(-std=c++2a)

file(GLOB_RECURSE TARGETS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_library(hnll_physics STATIC ${TARGETS})

include($ENV{HNLL_ENGN}/include.cmake)
include_common_dependencies(hnll_physics)
if (APPLE)
    include_mac_dependencies(hnll_physics)
endif(APPLE)