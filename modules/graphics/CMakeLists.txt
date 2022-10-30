set(TARGETS
        src/systems/grid_rendering_system.cpp
        src/systems/wire_frustum_rendering_system.cpp
        src/systems/mesh_rendering_system.cpp
        src/systems/point_light.cpp
        src/buffer.cpp
        src/camera.cpp
        src/descriptor_set_layout.cpp
        src/device.cpp
        src/engine.cpp
        src/mesh_model.cpp
        src/pipeline.cpp
        src/renderer.cpp
        src/swap_chain.cpp
        src/window.cpp
        ../../include/graphics/acceleration_structure.hpp ../../applications/ray_tracing/ray_traced_triangle/introducing_ray_tracing.cpp)
add_library(hnll_graphics STATIC ${TARGETS})

target_include_directories(hnll_graphics PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    $ENV{VULKAN_DIR}/include/
    ${PROJECT_SOURCE_DIR}/submodules/tiny_obj_loader)


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
        ../../submodules/nvvk_extensions/extensions_vk.cpp src/acceleration_structure.cpp)

add_executable(introduce_ray_tracing ${TARGETS})

target_include_directories(introduce_ray_tracing PUBLIC
        $ENV{HNLL_ENGN}/include
        $ENV{HNLL_ENGN}/submodules/tiny_obj_loader
        $ENV{HNLL_ENGN}/submodules/glfw
        $ENV{HNLL_ENGN}/submodules/nvvk_extensions
        )

target_link_directories(introduce_ray_tracing PUBLIC $ENV{VULKAN_DIR}/lib)

target_link_libraries(introduce_ray_tracing PUBLIC
        libvulkan.so.1.3.224
        glfw
        )