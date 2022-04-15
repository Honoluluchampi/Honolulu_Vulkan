file(GLOB_RECURSE TARGETS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

# Honolulu Imgui Engine
add_library(hie STATIC ${TARGETS})

# add external libraries
target_include_directories(hie PUBLIC
                           # hie
                           ${CMAKE_CURRENT_SOURCE_DIR}/include
                           # hge
                           # ${PROJECT_SOURCE_DIR}/honolulu_game_engine/include
                           # hve
                           ${PROJECT_SOURCE_DIR}/honolulu_vulkan_engine/include
                           # imgui
                           $ENV{IMGUI_DIR}
                           # glfw
                           /usr/include
                           # vulkan
                           $ENV{VULKAN_DIR}/include
)