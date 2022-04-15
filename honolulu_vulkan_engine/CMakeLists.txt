file(GLOB_RECURSE TARGETS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_library(hve STATIC ${TARGETS})

# add external libraries
target_include_directories(hve PUBLIC 
                           # hve
                           ${CMAKE_CURRENT_SOURCE_DIR}/include
                           # hge
                           ${PROJECT_SOURCE_DIR}/honolulu_game_engine/include
                           # glfw
                           /usr/include
                           # vulkan
                           /home/honolulu/programs/external_libraries/vulkanSDK/x86_64/include
)

target_link_libraries(hve /usr/lib)