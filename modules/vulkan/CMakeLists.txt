file(GLOB_RECURSE TARGETS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_library(hnll_vulkan STATIC ${TARGETS})

target_include_directories(hnll_vulkan PUBLIC
    # hnll
    ${PROJECT_SOURCE_DIR}/include
    # vulkan
    $ENV{VULKAN_DIR}/include
)

if (APPLE)
    target_include_directories(hnll_vulkan PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    $ENV{GLFW_DIR}/include
    $ENV{GLM_DIR}/include
    $ENV{VULKAN_DIR}/include
    )
endif (APPLE)