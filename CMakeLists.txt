# c++20 is available in VERSION 3.16 or later
cmake_minimum_required(VERSION 3.16)

# set the project name and its version
project(app VERSION 1.0)

# this header file passes the version number to the source code
# configure_file(testConfig.h.in testConfig.h)

# specify the c++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "-g3 -O0")

# add modules as libraries
add_subdirectory(modules/game)
add_subdirectory(modules/gui)
add_subdirectory(modules/graphics)
add_subdirectory(modules/utils)

# add the executable
add_executable(app main.cpp)

target_link_directories(app PUBLIC
    $ENV{VULKAN_DIR}/lib
)
if (APPLE)
    target_link_directories(app PUBLIC
        $ENV{VULKAN_DIR}/lib
        $ENV{GLFW_DIR}/lib
    )
endif (APPLE)

target_link_libraries(app PUBLIC
    hnll_game
    hnll_gui
    hnll_graphics
    hnll_utils
    vulkan
    glfw
#    X11
#    Xtst
)

# specify the include directories
# adding "${PROJECT_BINARY_TREE" is necessary when you want to find TutofrialConfig.h
# target_include_directories(test PUBLIC ${PROJECT_BINARY_DIR})

# install destination
# install(TARGETS app DESTINATION bin)
# install(FILES ${PROJECT_BINARY_DIR}/testConfig.h DESTINATION include)