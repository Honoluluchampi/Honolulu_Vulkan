file(GLOB_RECURSE TARGETS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

# Honolulu Imgui Engine
add_library(hnll_gui STATIC ${TARGETS})

if (APPLE)
    target_include_directories(hnll_gui PUBLIC
        ${PROJECT_SOURCE_DIR}/include
        $ENV{GLFW_DIR}/include
        $ENV{GLM_DIR}/include
        ${PROJECT_SOURCE_DIR}/submodules/imgui
        $ENV{VULKAN_DIR}/include
    )
elseif (UNIX)
    target_include_directories(hnll_gui PUBLIC
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/submodules/imgui
        $ENV{VULKAN_DIR}/include
    )
endif (APPLE)