set(TARGETS
        src/buffer.cpp
        src/camera.cpp
        src/descriptor_set_layout.cpp
        src/device.cpp
        ../game/src/modules/graphics_engine.cpp
        src/mesh_model.cpp
        src/meshlet_model.cpp
        src/pipeline.cpp
        src/renderer.cpp
        src/swap_chain.cpp
        src/window.cpp
        src/utils.cpp
        src/skinning_mesh_model.cpp
        src/skinning_utils.cpp
        src/frame_anim_mesh_model.cpp
        src/frame_anim_meshlet_model.cpp
        src/descriptor_set.cpp
        src/frame_anim_utils.cpp
        $ENV{HNLL_ENGN}/submodules/extensions/ray_tracing_extensions.cpp
        )

add_library(hnll_graphics STATIC ${TARGETS})

include($ENV{HNLL_ENGN}/include.cmake)
include_common_dependencies(hnll_graphics)
if (APPLE)
    include_mac_dependencies(hnll_graphics)
endif(APPLE)

# RAY TRACING SAMPLE
cmake_minimum_required(VERSION 3.16)
project(introduce_ray_tracing VERSION 1.0)

# specify the c++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "-g3 -O0")

set(TARGETS
        src/buffer.cpp
        src/camera.cpp
        src/descriptor_set_layout.cpp
        src/device.cpp
        src/mesh_model.cpp
        src/pipeline.cpp
        src/renderer.cpp
        src/swap_chain.cpp
        src/window.cpp
        src/acceleration_structure.cpp
        src/meshlet_model.cpp
        ../../submodules/extensions/ray_tracing_extensions.cpp)

add_executable(introduce_ray_tracing ${TARGETS})

include($ENV{HNLL_ENGN}/include.cmake)
include_common_dependencies(introduce_ray_tracing)
if (APPLE)
    include_mac_dependencies(introduce_ray_tracing)
endif(APPLE)

target_link_libraries(introduce_ray_tracing PUBLIC
    libvulkan.so.1.3.224
    glfw
)