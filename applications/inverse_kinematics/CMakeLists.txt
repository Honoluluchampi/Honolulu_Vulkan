cmake_minimum_required(VERSION 3.16)
project(inverse_kinematics VERSION 1.0)
project(inverse_kinematics_test)

# specify the c++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "-g3 -O0")

# build engine
add_subdirectory($ENV{HNLL_ENGN}/ $ENV{HNLL_ENGN}/build)

# build inward-kinematics
#file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
set(SOURCES src/main.cpp src/app.cpp src/control_point.cpp src/drag_manager.cpp src/bone.cpp)
add_executable(inverse_kinematics ${SOURCES})
target_include_directories(inverse_kinematics PUBLIC $ENV{HNLL_ENGN}/include include)
target_link_libraries(inverse_kinematics PUBLIC hnll_engine)

# build test
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
enable_testing()
set(TEST_SOURCES test/test.cpp src/app.cpp src/control_point.cpp src/drag_manager.cpp src/bone.cpp)
add_executable(inverse_kinematics_test ${TEST_SOURCES})
if (APPLE)
    target_include_directories(inverse_kinematics_test PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        $ENV{HNLL_ENGN}/include
        ${GOOGLE_TEST_DIRECTORY}/include)
    target_link_directories(inverse_kinematics_test PUBLIC ${GOOGLE_TEST_DIRECTORY}/lib)
elseif (UNIX)
    target_include_directories(inverse_kinematics_test PUBLIC
        $ENV{HNLL_ENGN}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include)
endif (APPLE)
target_link_libraries(inverse_kinematics_test hnll_engine gtest gtest_main pthread)