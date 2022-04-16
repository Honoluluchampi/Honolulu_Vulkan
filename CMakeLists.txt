# c++20 is available in VERSION 3.16 or later
cmake_minimum_required(VERSION 3.16)

# set the project name and its version
project(app VERSION 1.0)

# this header file passes the version number to the source code
# configure_file(testConfig.h.in testConfig.h)

# specify the c++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# add subdirectory's library (this call should corresponds to the mathlib's "add_library")
add_subdirectory(honolulu_vulkan_engine)
add_subdirectory(honolulu_imgui_engine)
add_subdirectory(honolulu_game_engine)

# add the executable
add_executable(app main.cpp)

target_link_libraries(app PUBLIC hge hie hve $ENV{VULKAN_DIR}/lib/libvulkan.so.1.3.204 $ENV{VULKAN_DIR}/lib/libvulkan.so.1 /usr/lib/x86_64-linux-gnu/libglfw.so.3.3)

# specify the include directories
# adding "${PROJECT_BINARY_TREE" is necessary when you want to find TutofrialConfig.h
# target_include_directories(test PUBLIC ${PROJECT_BINARY_DIR})

# install destination
install(TARGETS app DESTINATION bin)
# install(FILES ${PROJECT_BINARY_DIR}/testConfig.h DESTINATION include)