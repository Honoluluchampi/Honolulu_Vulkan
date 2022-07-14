file(GLOB_RECURSE TARGETS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_library(hnll_game STATIC ${TARGETS})

# add external libraries
target_include_directories(hnll_game PUBLIC
    # hnll
    ${PROJECT_SOURCE_DIR}/include
    # imgui
    $ENV{IMGUI_DIR}
    # vulkan
    $ENV{VULKAN_DIR}/include
)

if (APPLE)
    target_include_directories(hnll_game PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    $ENV{GLFW_DIR}/include
    $ENV{GLM_DIR}/include
    $ENV{IMGUI_DIR}
    $ENV{VULKAN_DIR}/include
    )
endif (APPLE)