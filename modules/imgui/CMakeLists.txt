file(GLOB_RECURSE TARGETS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

# Honolulu Imgui Engine
add_library(hnll_imgui STATIC ${TARGETS})

if (APPLE)
    target_include_directories(hnll_imgui PUBLIC
        ${PROJECT_SOURCE_DIR}/include
        $ENV{GLFW_DIR}/include
        $ENV{GLM_DIR}/include
        $ENV{IMGUI_DIR}/
        $ENV{VULKAN_DIR}/include
    )
elseif (UNIX)
    target_include_directories(hnll_imgui PUBLIC
        ${PROJECT_SOURCE_DIR}/include
        $ENV{IMGUI_DIR}/
        $ENV{VULKAN_DIR}/include
    )
endif (APPLE)