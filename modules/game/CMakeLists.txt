file(GLOB_RECURSE TARGETS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_library(hnll_game STATIC ${TARGETS})

include($ENV{HNLL_ENGN}/include.cmake)
include_common_dependencies(hnll_graphics)
if (APPLE)
    include_mac_dependencies(hnll_graphics)
endif(APPLE)