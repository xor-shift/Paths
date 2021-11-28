cmake_minimum_required(VERSION 3.21)
project(Paths)

set(CMAKE_CXX_STANDARD 23)

### CONAN STUFF & PACKAGES ###

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CONAN_COMPILER clang)
    set(CONAN_COMPILER_VERSION 13)

    set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -mllvm -asan-use-private-alias=1")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CONAN_COMPILER gcc)
    set(CONAN_COMPILER_VERSION 11.1)
else ()
endif ()

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

find_package(OpenMP REQUIRED)

#set(PATHS_USE_POLLY)

### FLAGS ###

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall -Wextra")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-parameter")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-gnu-zero-variadic-macro-arguments")
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

add_library(${PATHS_LIB_NAME}_Maths SHARED
        lib/maths/include/maths/maths.hpp
        lib/maths/include/maths/matrix.hpp
        lib/maths/include/maths/matvec.hpp
        lib/maths/include/maths/rand.hpp
        lib/maths/include/maths/vector.hpp)
target_include_directories(${PATHS_LIB_NAME}_Maths PUBLIC lib/maths/include)
set_target_properties(${PATHS_LIB_NAME}_Maths PROPERTIES LINKER_LANGUAGE CXX)
target_compile_options(${PATHS_LIB_NAME}_Maths INTERFACE -fno-exceptions)

add_library(${PATHS_LIB_NAME}_Utils INTERFACE
        lib/utils/include/utils/buf_chan.hpp
        lib/utils/include/utils/circular_buffer.hpp
        lib/utils/include/utils/pointer_iterator.hpp
        lib/utils/include/utils/spin_lock.hpp
        lib/utils/include/utils/utils.hpp
        lib/utils/include/utils/wait_group.hpp
        lib/utils/include/utils/worker_pool.hpp)
target_include_directories(${PATHS_LIB_NAME}_Utils INTERFACE lib/utils/include)
target_compile_options(${PATHS_LIB_NAME}_Utils INTERFACE -fno-exceptions)

add_library(${PATHS_LIB_NAME} SHARED
        thirdparty/lodepng/lodepng.cpp
        thirdparty/tinyexr/tinyexr.cc

        lib/gfx/include/gfx/image/exporters/exr.hpp
        lib/gfx/include/gfx/image/exporters/png.hpp
        lib/gfx/include/gfx/image/filter.hpp
        lib/gfx/include/gfx/image/image.hpp
        lib/gfx/src/image/exporters/exr.cpp
        lib/gfx/src/image/exporters/png.cpp

        lib/gfx/include/gfx/integrator/averager.hpp
        lib/gfx/src/gfx/integrator/averager.cpp
        lib/gfx/include/gfx/integrator/integrator.hpp
        lib/gfx/include/gfx/integrator/sampler.hpp
        lib/gfx/src/gfx/integrator/mc.cpp
        lib/gfx/src/gfx/integrator/whitted.cpp
        lib/gfx/include/gfx/integrator/sampler_wrapper.hpp
        lib/gfx/include/gfx/integrator/stub_integrator.hpp

        lib/gfx/include/gfx/scene/bvh.hpp
        lib/gfx/include/gfx/scene/scene.hpp
        lib/gfx/include/gfx/scene/store.hpp
        lib/gfx/include/gfx/scene/thinbvh.hpp

        lib/gfx/include/gfx/shape/shape.hpp
        lib/gfx/include/gfx/shape/shapes.hpp

        lib/gfx/include/gfx/stl/binary.hpp
        lib/gfx/src/gfx/stl/binary.cpp
        lib/gfx/include/gfx/stl/common.hpp

        lib/gfx/include/gfx/camera.hpp
        lib/gfx/src/gfx/camera.cpp
        lib/gfx/include/gfx/color.hpp
        lib/gfx/include/gfx/common.hpp
        lib/gfx/include/gfx/material.hpp
        lib/gfx/include/gfx/ray.hpp
        )

foreach (shape aabox disc plane sphere triangle)
    target_sources(${PATHS_LIB_NAME} PUBLIC lib/gfx/include/gfx/shape/${shape}.hpp)
endforeach ()

target_include_directories(${PATHS_LIB_NAME} PUBLIC
        lib/gfx/include
        thirdparty/lodepng
        thirdparty/tinyexr)
conan_target_link_libraries(${PATHS_LIB_NAME})
target_link_libraries(${PATHS_LIB_NAME} ${PATHS_LIB_NAME}_Maths ${PATHS_LIB_NAME}_Utils pthread omp fmt)
target_compile_options(${PATHS_LIB_NAME} PUBLIC -fno-exceptions)

### TESTS ###

add_subdirectory(thirdparty/googletest)

add_executable(${PATHS_TESTS_NAME}
        tests/test_test.cpp
        tests/test_maths.cpp)
target_include_directories(${PATHS_TESTS_NAME} PUBLIC thirdparty/googletest/googletest/include)
target_link_libraries(${PATHS_TESTS_NAME} ${PATHS_LIB_NAME} gtest gtest_main)

add_executable(${PATHS_TESTS_NAME}_distrib
        tests/distribTest.cpp)
target_link_libraries(${PATHS_TESTS_NAME}_distrib ${PATHS_LIB_NAME})

### MAIN ###

add_executable(${PATHS_EXEC_NAME}
        paths/main.cpp)
target_include_directories(${PATHS_EXEC_NAME} PUBLIC
        lib/gfx/include)
target_sources(${PATHS_EXEC_NAME} PUBLIC
        paths/lua.hpp
        paths/lua.cpp

        paths/luaCompat.hpp
        paths/luaCompat$camera.cpp
        paths/luaCompat$integrator.cpp
        paths/luaCompat$matrix.cpp
        paths/luaCompat$scene.cpp
        paths/luaCompat$store.cpp
        paths/luaCompat$vector.cpp
        )
conan_target_link_libraries(${PATHS_EXEC_NAME})
target_link_libraries(${PATHS_EXEC_NAME} ${PATHS_LIB_NAME}_Maths ${PATHS_LIB_NAME})
target_compile_options(${PATHS_EXEC_NAME} PUBLIC
        -fno-exceptions
        -fopenmp)
set_source_files_properties(paths/lua.cpp PROPERTIES COMPILE_FLAGS -fexceptions)
set_source_files_properties(paths/luaCompat$camera.cpp PROPERTIES COMPILE_FLAGS -fexceptions)
set_source_files_properties(paths/luaCompat$integrator.cpp PROPERTIES COMPILE_FLAGS -fexceptions)
set_source_files_properties(paths/luaCompat$matrix.cpp PROPERTIES COMPILE_FLAGS -fexceptions)
set_source_files_properties(paths/luaCompat$scene.cpp PROPERTIES COMPILE_FLAGS -fexceptions)
set_source_files_properties(paths/luaCompat$store.cpp PROPERTIES COMPILE_FLAGS -fexceptions)
set_source_files_properties(paths/luaCompat$vector.cpp PROPERTIES COMPILE_FLAGS -fexceptions)
conan_target_link_libraries(${PATHS_EXEC_NAME})
