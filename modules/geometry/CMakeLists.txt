add_definitions(-std=c++2a)

file(GLOB_RECURSE TARGETS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_library(hnll_geometry STATIC ${TARGETS})

target_include_directories(hnll_geometry PUBLIC
        $ENV{HNLL_ENGN}/include
        ${PROJECT_SOURCE_DIR}/submodules/tiny_obj_loader
        $ENV{GLM_DIR}
        )