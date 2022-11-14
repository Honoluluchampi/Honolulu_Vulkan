file(GLOB_RECURSE TARGETS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_library(hnll_game STATIC ${TARGETS})

if (APPLE)
    target_include_directories(hnll_game PUBLIC
        ${PROJECT_SOURCE_DIR}/include
        $ENV{GLFW_DIR}/include
        $ENV{GLM_DIR}/include
        ${PROJECT_SOURCE_DIR}/submodules/imgui
        $ENV{VULKAN_DIR}/include
    )
elseif (UNIX)
    target_include_directories(hnll_game PUBLIC
        $ENV{HNLL_ENGN}/include
        $ENV{VULKAN_DIR}/include
        $ENV{HNLL_ENGN}/submodules/imgui
    )
endif (APPLE)