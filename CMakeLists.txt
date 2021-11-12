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
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-parameter -Wno-gnu-zero-variadic-macro-arguments")
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
        lib/utils/include/utils/spin_lock.hpp)
target_include_directories(${PATHS_LIB_NAME}_Utils INTERFACE lib/utils/include)
target_compile_options(${PATHS_LIB_NAME}_Utils INTERFACE -fno-exceptions)

add_library(${PATHS_LIB_NAME}
        thirdparty/lodepng/lodepng.cpp
        thirdparty/tinyexr/tinyexr.cc

        lib/gfx/include/gfx/image/exporters/exr.hpp
        lib/gfx/include/gfx/image/exporters/png.hpp
        lib/gfx/include/gfx/image/color.hpp
        lib/gfx/include/gfx/image/filter.hpp
        lib/gfx/include/gfx/image/image.hpp

        lib/gfx/src/image/exporters/exr.cpp
        lib/gfx/src/image/exporters/png.cpp
        lib/gfx/src/image/image.cpp
        )
target_include_directories(${PATHS_LIB_NAME} PUBLIC
        #lib/maths/include
        lib/gfx/include
        #lib/utils/include
        thirdparty/lodepng
        thirdparty/tinyexr)
conan_target_link_libraries(${PATHS_LIB_NAME})
target_link_libraries(${PATHS_LIB_NAME} ${PATHS_LIB_NAME}_Maths ${PATHS_LIB_NAME}_Utils pthread omp fmt)
target_compile_options(${PATHS_LIB_NAME} PRIVATE -fno-exceptions)

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
        paths/main.cpp)
target_include_directories(${PATHS_EXEC_NAME} PRIVATE
        lib/gfx/include)
conan_target_link_libraries(${PATHS_EXEC_NAME})
target_link_libraries(${PATHS_EXEC_NAME} ${PATHS_LIB_NAME}_Maths ${PATHS_LIB_NAME})
target_compile_options(${PATHS_EXEC_NAME} PRIVATE
        -fno-exceptions
        -fopenmp)
