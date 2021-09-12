cmake_minimum_required(VERSION 3.21)
project(Paths)

find_package(SFML 2.5 REQUIRED COMPONENTS graphics window network audio system)
find_package(ImGui-SFML REQUIRED)
find_package(OpenMP REQUIRED)

set(CMAKE_CXX_STANDARD 23)

add_subdirectory(lib)
add_subdirectory(thirdparty/gcem)

set(Paths_headers
        )

set(Paths_sources
        paths/main.cpp
        )

add_executable(Paths ${Paths_headers} ${Paths_sources})

target_include_directories(Paths PRIVATE
        lib/gfx/include
        ./thirdparty/gcem/include
        #${SFML_INCLUDE_DIR}
        )

target_link_libraries(Paths
        pthread
        omp
        fmt
        sfml-graphics sfml-window sfml-network sfml-audio sfml-system
        ImGui-SFML::ImGui-SFML
        paths_gfx
        paths_maths
        paths_utils
        )

target_compile_options(Paths PRIVATE
        -Wall
        -pedantic
        -fopenmp
        -march=native
        -mtune=native
        -funsafe-math-optimizations
        )


add_executable(${PROJECT_NAME}_distrib_tests
        paths/distribTest.cpp
        )
target_include_directories(${PROJECT_NAME}_distrib_tests PRIVATE thirdparty/implot)
target_link_libraries(${PROJECT_NAME}_distrib_tests
        pthread
        omp
        fmt
        paths_maths)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall -Wextra")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -mtune=native")
if ((CMAKE_BUILD_TYPE MATCHES Debug) OR (CMAKE_BUILD_TYPE MATCHES RelWithDebInfo))
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize=undefined")
    set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize=undefined")

    target_compile_options(Paths PRIVATE
            -g -fno-omit-frame-pointer)

    target_link_options(Paths PRIVATE)
endif ()
