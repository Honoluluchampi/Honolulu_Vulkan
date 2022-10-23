file(GLOB_RECURSE TARGETS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_library(hnll_graphics STATIC ${TARGETS})

target_include_directories(hnll_graphics PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    $ENV{VULKAN_DIR}/include/
    ${PROJECT_SOURCE_DIR}/submodules/tiny_obj_loader)