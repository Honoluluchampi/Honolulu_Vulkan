# RAY TRACING SAMPLE
cmake_minimum_required(VERSION 3.16)
project(scene_objects VERSION 1.0)

# specify the c++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "-g3 -O0")

set(TARGETS
        scene_objects.cpp
        $ENV{HNLL_ENGN}/modules/graphics/src/buffer.cpp
        $ENV{HNLL_ENGN}/modules/graphics/src/camera.cpp
        $ENV{HNLL_ENGN}/modules/graphics/src/descriptor_set_layout.cpp
        $ENV{HNLL_ENGN}/modules/graphics/src/device.cpp
        $ENV{HNLL_ENGN}/modules/graphics/src/mesh_model.cpp
        $ENV{HNLL_ENGN}/modules/graphics/src/pipeline.cpp
        $ENV{HNLL_ENGN}/modules/graphics/src/renderer.cpp
        $ENV{HNLL_ENGN}/modules/graphics/src/swap_chain.cpp
        $ENV{HNLL_ENGN}/modules/graphics/src/window.cpp
        ../../../submodules/nvvk_extensions/extensions_vk.cpp
        $ENV{HNLL_ENGN}/modules/graphics/src/acceleration_structure.cpp)

add_executable(scene_objects ${TARGETS})

target_include_directories(scene_objects PUBLIC
        $ENV{HNLL_ENGN}/include
        $ENV{HNLL_ENGN}/submodules/tiny_obj_loader
        $ENV{HNLL_ENGN}/submodules/glfw
        $ENV{HNLL_ENGN}/submodules/nvvk_extensions
        )

target_link_directories(scene_objects PUBLIC $ENV{VULKAN_DIR}/lib)

target_link_libraries(scene_objects PUBLIC
        libvulkan.so.1.3.224
        glfw
        )