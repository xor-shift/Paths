cmake_minimum_required(VERSION 3.21)
project(libpaths)
set(CMAKE_CXX_STANDARD 23)

set(CONAN_COMPILER clang)
set(CONAN_COMPILER_VERSION 12)
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_library(${PROJECT_NAME}
        deps/lodepng/lodepng.cpp
        deps/tinyexr/tinyexr.cc

        src/gfx/scene/bvh.cpp
        src/gfx/obj/obj.cpp
        src/gfx/obj/common.cpp
        src/gfx/scene/scene.cpp
        src/gfx/stl/binary.cpp
        src/gfx/stl/common.cpp
        src/gfx/image.cpp
        )
target_include_directories(${PROJECT_NAME} PUBLIC
        include
        deps/lodepng
        deps/tinyexr
        )
conan_target_link_libraries(${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME}
        pthread omp fmt
        )

add_subdirectory(tests)
