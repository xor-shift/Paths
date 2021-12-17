cmake_minimum_required(VERSION 3.21)
project(Paths)

set(CMAKE_CXX_STANDARD 23)

### CONAN STUFF & PACKAGES ###

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CONAN_COMPILER clang)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CONAN_COMPILER gcc)
else ()
endif ()

set(CONAN_COMPILER_VERSION ${CMAKE_CXX_COMPILER_VERSION})

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

### FLAGS ###

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall -Wextra -Wno-nonnull")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-parameter")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -mtune=native -funsafe-math-optimizations")
if ((CMAKE_BUILD_TYPE MATCHES Debug) OR (CMAKE_BUILD_TYPE MATCHES RelWithDebInfo))
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize=undefined")
    set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -fsanitize=address -fsanitize=undefined")
endif ()

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mllvm -polly")

set(PATHS_LIB_NAME Lib${PROJECT_NAME})
set(PATHS_EXEC_NAME ${PROJECT_NAME})
set(PATHS_TESTS_NAME ${PROJECT_NAME}Tests)
set(PATHS_BENCH_NAME ${PROJECT_NAME}Bench)

### LIB ###

add_library(${PATHS_LIB_NAME} STATIC
        thirdparty/lodepng/lodepng.cpp
        thirdparty/tinyexr/tinyexr.cc

        Lib/Include/Maths/Maths.hpp
        Lib/Include/Maths/Matrix.hpp
        Lib/Include/Maths/MatVec.hpp
        Lib/Include/Maths/Random.hpp
        Lib/Include/Maths/Vector.hpp

        Lib/Include/Utils/BufferedChannel.hpp
        Lib/Include/Utils/CircularBuffer.hpp
        Lib/Include/Utils/PointerIterator.hpp
        Lib/Include/Utils/SpinLock.hpp
        Lib/Include/Utils/Utils.hpp
        Lib/Include/Utils/WaitGroup.hpp
        Lib/Include/Utils/WorkerPool.hpp

        Lib/Include/Paths/Image/Exporters/EXRExporter.hpp
        Lib/Include/Paths/Image/Exporters/PNGExporter.hpp
        Lib/Include/Paths/Image/Filter.hpp
        Lib/Include/Paths/Image/Image.hpp
        Lib/Src/Paths/Image/Exporters/EXRExporter.cpp
        Lib/Src/Paths/Image/Exporters/PNGExporter.cpp

        Lib/Include/Paths/Integrator/Sampler/Albedo.hpp
        Lib/Include/Paths/Integrator/Averager.hpp
        Lib/Include/Paths/Integrator/Integrator.hpp
        Lib/Include/Paths/Integrator/Sampler/MonteCarlo.hpp
        Lib/Include/Paths/Integrator/Sampler/SamplerWrapper.hpp
        Lib/Include/Paths/Integrator/Sampler/Statistics.hpp
        Lib/Include/Paths/Integrator/Sampler/Whitted.hpp
        Lib/Src/Paths/Integrator/Sampler/Albedo.cpp
        Lib/Src/Paths/Integrator/Averager.cpp
        Lib/Src/Paths/Integrator/Sampler/MonteCarlo.cpp
        Lib/Src/Paths/Integrator/Sampler/Statistics.cpp
        Lib/Src/Paths/Integrator/Sampler/Whitted.cpp

        Lib/Include/Paths/Scene/Tree.hpp
        Lib/Include/Paths/Scene/Scene.hpp
        Lib/Include/Paths/Scene/Store.hpp
        Lib/Include/Paths/Scene/TBVH.hpp
        Lib/Include/Paths/Scene/ThinBVH.hpp
        Lib/Include/Paths/Scene/Traversal.hpp

        Lib/Include/Paths/Shape/Shape.hpp
        Lib/Include/Paths/Shape/Shapes.hpp

        Lib/Include/Paths/Lua/Lua.hpp
        Lib/Include/Paths/Lua/LuaCompat.hpp
        Lib/Src/Paths/Lua/Lua.cpp
        Lib/Src/Paths/Lua/LuaCompat$Camera.cpp
        Lib/Src/Paths/Lua/LuaCompat$Image.cpp
        Lib/Src/Paths/Lua/LuaCompat$Integrator.cpp
        Lib/Src/Paths/Lua/LuaCompat$Matrix.cpp
        Lib/Src/Paths/Lua/LuaCompat$Ray.cpp
        Lib/Src/Paths/Lua/LuaCompat$Scene.cpp
        Lib/Src/Paths/Lua/LuaCompat$Store.cpp
        Lib/Src/Paths/Lua/LuaCompat$Vector.cpp

        Lib/Include/Paths/STL/Binary.hpp
        Lib/Include/Paths/STL/Common.hpp
        Lib/Src/Paths/STL/Binary.cpp

        Lib/Include/Paths/Camera.hpp
        Lib/Src/Paths/Camera.cpp
        Lib/Include/Paths/Color.hpp
        Lib/Include/Paths/Common.hpp
        Lib/Include/Paths/Material/material.hpp
        Lib/Include/Paths/Ray.hpp
        )

foreach (shape AxisAlignedBox Disc Plane Sphere Triangle)
    target_sources(${PATHS_LIB_NAME} PUBLIC Lib/Include/Paths/Shape/${shape}.hpp)
endforeach ()

file(GLOB lua_compat_files "Lib/Src/Paths/Lua/LuaCompat$*.cpp")
foreach(file ${lua_compat_files})
    set_source_files_properties(${file} PROPERTIES COMPILE_FLAGS -fexceptions)
endforeach()
set_source_files_properties(Lib/Src/Paths/Lua/Lua.cpp PROPERTIES COMPILE_FLAGS -fexceptions)

target_include_directories(${PATHS_LIB_NAME} PUBLIC
        Lib/Include
        thirdparty/lodepng
        thirdparty/tinyexr)
conan_target_link_libraries(${PATHS_LIB_NAME})
target_link_libraries(${PATHS_LIB_NAME} pthread omp fmt)
target_compile_options(${PATHS_LIB_NAME} PUBLIC -fno-exceptions)

### TESTS ###

add_subdirectory(thirdparty/googletest)

add_executable(${PATHS_TESTS_NAME}
        Paths/Tests/test_test.cpp
        Paths/Tests/test_maths.cpp
        Paths/Tests/test_prng.cpp)
target_include_directories(${PATHS_TESTS_NAME} PUBLIC thirdparty/googletest/googletest/include)
target_link_libraries(${PATHS_TESTS_NAME} ${PATHS_LIB_NAME} gtest gtest_main)

add_executable(${PATHS_TESTS_NAME}_distrib
        Paths/Tests/distribTest.cpp)
target_link_libraries(${PATHS_TESTS_NAME}_distrib ${PATHS_LIB_NAME})


### BENCHMARKS ###

add_subdirectory(thirdparty/benchmark)

add_executable(${PATHS_BENCH_NAME}
        Paths/Benchmarks/rand.cpp)
target_include_directories(${PATHS_BENCH_NAME} PUBLIC thirdparty/benchmark/include)
target_link_libraries(${PATHS_BENCH_NAME} ${PATHS_LIB_NAME} benchmark)

### MAIN ###

add_executable(${PATHS_EXEC_NAME} Paths/Main/main.cpp)
target_include_directories(${PATHS_EXEC_NAME} PUBLIC Lib/Include)

conan_target_link_libraries(${PATHS_EXEC_NAME})
target_link_libraries(${PATHS_EXEC_NAME} ${PATHS_LIB_NAME})
target_compile_options(${PATHS_EXEC_NAME} PUBLIC
        -fno-exceptions
        -fopenmp)
set_source_files_properties(Lib/Src/Paths/Lua/Lua.cpp PROPERTIES COMPILE_FLAGS -fexceptions)
