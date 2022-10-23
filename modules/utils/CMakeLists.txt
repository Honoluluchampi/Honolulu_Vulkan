file(GLOB_RECURSE TARGETS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

# Honolulu utils Engine
add_library(hnll_utils STATIC ${TARGETS})

target_include_directories(hnll_utils PUBLIC
    $ENV{HNLL_ENGN}/include
)