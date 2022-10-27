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
        )
add_library(hnll_graphics STATIC ${TARGETS})

target_include_directories(hnll_graphics PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    $ENV{VULKAN_DIR}/include/
    ${PROJECT_SOURCE_DIR}/submodules/tiny_obj_loader)


# RAY TRACING SAMPLE
project(introduce_ray_tracing VERSION 1.0)

# specify the c++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "-g3 -O0")

