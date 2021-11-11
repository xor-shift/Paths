cmake_minimum_required(VERSION 3.21)
project(Paths)

set(CMAKE_CXX_STANDARD 23)

### CONAN STUFF & PACKAGES ###

set(CONAN_COMPILER clang)
set(CONAN_COMPILER_VERSION 12)
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

find_package(OpenMP REQUIRED)
add_subdirectory(libpaths)

#set(PATHS_USE_POLLY)

### FLAGS ###

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall -Wextra")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -mtune=native -funsafe-math-optimizations")
if ((CMAKE_BUILD_TYPE MATCHES Debug) OR (CMAKE_BUILD_TYPE MATCHES RelWithDebInfo))
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize=undefined")
    set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -fsanitize=address -fsanitize=undefined")
endif ()

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mllvm -polly")

### MAIN ###

add_executable(Paths
        paths/main.cpp
        paths/cameraSetup.cpp
        )
target_include_directories(Paths PRIVATE
        lib/gfx/include
        )
conan_target_link_libraries(Paths)
target_link_libraries(Paths
        libpaths
        )
target_compile_options(Paths PRIVATE
        -Wall -pedantic
        -fopenmp
        )
