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
        #${SFML_LIBRARIES}
        #${SFML_DEPENDENCIES}
        ImGui-SFML::ImGui-SFML
        paths_gfx
        )

target_compile_options(Paths PRIVATE
        -Wall
        -pedantic
        -fopenmp
        -march=native
        -mtune=native
        -funsafe-math-optimizations
        )

if ((CMAKE_BUILD_TYPE MATCHES Debug) OR (CMAKE_BUILD_TYPE MATCHES RelWithDebInfo))
    target_compile_options(Paths PRIVATE
            -O2
            -g -fno-omit-frame-pointer
            -fsanitize=address -fsanitize=undefined)

    target_link_options(Paths PRIVATE
            -rdynamic -fsanitize=address -fsanitize=undefined)
endif ()
