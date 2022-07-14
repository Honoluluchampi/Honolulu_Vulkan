file(GLOB_RECURSE TARGETS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_library(hge STATIC ${TARGETS})

# add external libraries
target_include_directories(hge PUBLIC 
                           # hge
                           ${CMAKE_CURRENT_SOURCE_DIR}/include
                           # hve
                           ${PROJECT_SOURCE_DIR}/honolulu_vulkan_engine/include
                           # hie
                           ${PROJECT_SOURCE_DIR}/honolulu_imgui_engine/include
                           # imgui
                           $ENV{IMGUI_DIR}
                           # glfw
                           /usr/include
                           # vulkan
                           $ENV{VULKAN_DIR}/include
)