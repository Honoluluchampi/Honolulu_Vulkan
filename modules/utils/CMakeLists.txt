file(GLOB_RECURSE TARGETS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(hnll_utils STATIC ${TARGETS})

include($ENV{HNLL_ENGN}/include.cmake)
include_common_dependencies(hnll_utils)
if (APPLE)
    include_mac_dependencies(hnll_utils)
endif(APPLE)