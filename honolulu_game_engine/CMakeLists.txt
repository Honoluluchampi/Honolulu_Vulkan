file(GLOB_RECURSE TARGETS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_library(hge STATIC ${TARGETS})

# add external libraries
target_include_directories(hge PUBLIC 
                           # hge
                           ${CMAKE_CURRENT_SOURCE_DIR}/include
                           #hve
                           ${PROJECT_SOURCE_DIR}/honolulu_vulkan_engine/include
                           # glfw
                           /usr/include
                           # vulkan
                           /home/honolulu/programs/external_libraries/vulkanSDK/x86_64/include
)

#target_link_libraries(hge /usr/lib)