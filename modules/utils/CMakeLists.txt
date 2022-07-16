file(GLOB_RECURSE TARGETS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

# Honolulu utils Engine
add_library(hnll_utils STATIC ${TARGETS})

target_include_directories(hnll_utils PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    $ENV{GLM_DIR}/include
)