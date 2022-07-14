file(GLOB_RECURSE TARGETS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_library(hnll_graphics STATIC ${TARGETS})

if (APPLE)
    target_include_directories(hnll_graphics PUBLIC
        ${PROJECT_SOURCE_DIR}/include
        $ENV{GLFW_DIR}/include
        $ENV{GLM_DIR}/include
        $ENV{VULKAN_DIR}/include/
        $ENV{TINY_OBJ_LOADER_DIR}
    )
elseif (UNIX)
    target_include_directories(hnll_graphics PUBLIC
        ${PROJECT_SOURCE_DIR}/include
        $ENV{VULKAN_DIR}/include
        $ENV{TINY_OBJ_LOADER_DIR}
    )
endif (APPLE)