cmake_minimum_required(VERSION 3.21)
project(Paths)

set(CMAKE_CXX_STANDARD 23)

### CONAN STUFF & PACKAGES ###

set(CONAN_COMPILER clang)
set(CONAN_COMPILER_VERSION 13)
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

find_package(OpenMP REQUIRED)

add_subdirectory(lib)

#set(PATHS_USE_POLLY)

### MAIN ###

add_executable(Paths
        paths/main.cpp
        paths/cameraSetup.cpp
        thirdparty/lodepng/lodepng.cpp
        thirdparty/tinyexr/tinyexr.cc
        )
target_include_directories(Paths PRIVATE
        lib/gfx/include
        thirdparty/lodepng
        thirdparty/tinyexr
        )
conan_target_link_libraries(Paths)
target_link_libraries(Paths
        pthread omp fmt
        paths_gfx paths_maths paths_utils
        )
target_compile_options(Paths PRIVATE
        -Wall -pedantic
        -fopenmp
        )

### DISTRIBUTION TESTS ###

add_executable(${PROJECT_NAME}_distrib_tests
        paths/distribTest.cpp
        )
target_include_directories(${PROJECT_NAME}_distrib_tests PRIVATE thirdparty/implot)
conan_target_link_libraries(Paths)
target_link_libraries(${PROJECT_NAME}_distrib_tests
        pthread omp fmt
        paths_maths
        )

### OTHER STUFF ###

if (DEFINED PATHS_USE_POLLY)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mllvm -polly")
endif ()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall -Wextra")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -mtune=native -funsafe-math-optimizations")
if ((CMAKE_BUILD_TYPE MATCHES Debug) OR (CMAKE_BUILD_TYPE MATCHES RelWithDebInfo))
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize=undefined")
    set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -fsanitize=address -fsanitize=undefined")

    target_compile_options(Paths PRIVATE
            -g -fno-omit-frame-pointer)

    target_link_options(Paths PRIVATE)
endif ()

if (UNIX)
    set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -lX11")
endif ()
