cmake_minimum_required(VERSION 3.21)
project(Paths)

set(CMAKE_CXX_STANDARD 23)

### CONAN STUFF & PACKAGES ###

set(CONAN_COMPILER clang)
set(CONAN_COMPILER_VERSION 12)
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

find_package(OpenMP REQUIRED)

#set(PATHS_USE_POLLY)

### FLAGS ###

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall -Wextra")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -mtune=native -funsafe-math-optimizations")
if ((CMAKE_BUILD_TYPE MATCHES Debug) OR (CMAKE_BUILD_TYPE MATCHES RelWithDebInfo))
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize=undefined")
    set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -fsanitize=address -fsanitize=undefined")
endif ()

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mllvm -polly")

set(PATHS_LIB_NAME Lib${PROJECT_NAME})
set(PATHS_EXEC_NAME ${PROJECT_NAME})
set(PATHS_TESTS_NAME ${PROJECT_NAME}Tests)

### LIB ###

add_library(${PATHS_LIB_NAME}
        thirdparty/lodepng/lodepng.cpp
        thirdparty/tinyexr/tinyexr.cc

        lib/src/gfx/scene/bvh.cpp
        lib/src/gfx/obj/obj.cpp
        lib/src/gfx/obj/common.cpp
        lib/src/gfx/scene/scene.cpp
        lib/src/gfx/stl/binary.cpp
        lib/src/gfx/stl/common.cpp
        lib/src/gfx/image.cpp)
target_include_directories(${PATHS_LIB_NAME} PUBLIC
        lib/include
        thirdparty/lodepng
        thirdparty/tinyexr)
conan_target_link_libraries(${PATHS_LIB_NAME})
target_link_libraries(${PATHS_LIB_NAME} pthread omp fmt)

### TESTS ###

add_subdirectory(thirdparty/googletest)

add_executable(${PATHS_TESTS_NAME}
        tests/test_test.cpp
        tests/test_maths.cpp)
target_include_directories(${PATHS_TESTS_NAME} PRIVATE thirdparty/googletest/googletest/include)
target_link_libraries(${PATHS_TESTS_NAME} ${PATHS_LIB_NAME} gtest gtest_main)

add_executable(${PATHS_TESTS_NAME}_distrib
        tests/distribTest.cpp)
target_link_libraries(${PATHS_TESTS_NAME}_distrib ${PATHS_LIB_NAME})

### MAIN ###

add_executable(${PATHS_EXEC_NAME}
        paths/main.cpp
        paths/cameraSetup.cpp)
target_include_directories(${PATHS_EXEC_NAME} PRIVATE
        lib/gfx/include)
conan_target_link_libraries(${PATHS_EXEC_NAME})
target_link_libraries(${PATHS_EXEC_NAME}
        Lib${PROJECT_NAME})
target_compile_options(${PATHS_EXEC_NAME} PRIVATE
        -fopenmp)
