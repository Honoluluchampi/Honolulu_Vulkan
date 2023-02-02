file(GLOB_RECURSE TARGETS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(hnll_gui STATIC ${TARGETS})

include($ENV{HNLL_ENGN}/include.cmake)
include_common_dependencies(hnll_gui)
if (APPLE)
    include_mac_dependencies(hnll_gui)
endif(APPLE)

# imgui backends needs its own directory as include directory
target_include_directories(hnll_gui PUBLIC
    $ENV{HNLL_ENGN}/submodules/imgui
)