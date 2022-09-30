add_definitions(-std=c++2a)

file(GLOB_RECURSE TARGETS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_library(hnll_geometry STATIC ${TARGETS})

target_include_directories(hnll_geometry PUBLIC
        $ENV{HNLL_ENGN}/include)

#---------------------------------------------
# build test
cmake_minimum_required(VERSION 3.16)
project(geometry_test)
enable_testing()
file(GLOB_RECURSE TEST_SRC ${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp)
add_executable(geometry_test ${TEST_SRC})

# for OS X
if (APPLE)
    # search brew's root
    execute_process(
            COMMAND brew --prefix
            RESULT_VARIABLE BREW
            OUTPUT_VARIABLE BREW_PREFIX
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    execute_process(
            COMMAND ls ${BREW_PREFIX}/Cellar/googletest
            RESULT_VARIABLE GOOGLE_TEST
            OUTPUT_VARIABLE GOOGLE_TEST_VERSION
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    set(GOOGLE_TEST_DIRECTORY ${BREW_PREFIX}/Cellar/googletest/${GOOGLE_TEST_VERSION})
endif (APPLE)

if (APPLE)
    target_include_directories(geometry_test PUBLIC
            # googletest
            ${GOOGLE_TEST_DIRECTORY}/include)
    target_link_directories(geometry_test PUBLIC build
            # googletest
            ${GOOGLE_TEST_DIRECTORY}/lib)
elseif (UNIX)
    target_include_directories(geometry_test PUBLIC $ENV{HNLL_ENGN}/include)
    target_link_directories(geometry_test PUBLIC build)
endif (APPLE)
target_link_libraries(geometry_test hnll_geometry gtest gtest_main pthread)