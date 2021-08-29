cmake_minimum_required(VERSION 3.21)
project(Paths)

find_package(SFML 2.5 REQUIRED COMPONENTS graphics window network audio system)
find_package(ImGui-SFML REQUIRED)
find_package(OpenMP REQUIRED)

set(CMAKE_CXX_STANDARD 23)

add_subdirectory(lib)

set(Paths_headers
        lib/include/gfx/concepts/integrator.hpp
        lib/include/gfx/concepts/shape.hpp

        lib/include/gfx/integrators/integrators.hpp
        lib/include/gfx/integrators/samplerWrapper.hpp

        lib/include/gfx/samplers/whitter.hpp

        lib/include/gfx/shapes/aabox.hpp
        lib/include/gfx/shapes/plane.hpp
        lib/include/gfx/shapes/shapes.hpp
        lib/include/gfx/shapes/triangle.hpp

        lib/include/gfx/camera.hpp
        lib/include/gfx/definitions.hpp
        lib/include/gfx/image.hpp
        lib/include/gfx/material.hpp
        lib/include/gfx/ray.hpp
        lib/include/gfx/scene.hpp
        lib/include/gfx/spectrum.hpp

        #lib/include/math/math.hpp

        lib/include/util/util.hpp

        )

set(Paths_sources
        lib/src/gfx/concepts/integrator.cpp

        lib/src/gfx/integrators/samplerWrapper.cpp

        lib/src/gfx/samplers/whitter.cpp

        lib/src/gfx/shapes/aabox.cpp
        lib/src/gfx/shapes/plane.cpp
        lib/src/gfx/shapes/shape.cpp
        lib/src/gfx/shapes/triangle.cpp

        lib/src/gfx/camera.cpp
        lib/src/gfx/definitions.cpp
        lib/src/gfx/image.cpp
        lib/src/gfx/material.cpp
        lib/src/gfx/scene.cpp
        lib/src/gfx/spectrum.cpp

        #lib/src/math/math.cpp

        paths/main.cpp

        )

add_executable(Paths ${Paths_headers} ${Paths_sources})

target_include_directories(Paths PRIVATE
        ./lib/include
        ./thirdparty/gcem/include
        #${SFML_INCLUDE_DIR}
        )

target_link_libraries(Paths
        omp
        fmt
        sfml-graphics sfml-window sfml-network sfml-audio sfml-system
        #${SFML_LIBRARIES}
        #${SFML_DEPENDENCIES}
        ImGui-SFML::ImGui-SFML
        paths_maths
        )

target_compile_options(Paths PRIVATE
        -O2
        -Wall
        -pedantic
        -fopenmp)

if ((CMAKE_BUILD_TYPE MATCHES Debug) OR (CMAKE_BUILD_TYPE MATCHES RelWithDebInfo))
    target_compile_options(Paths PRIVATE
            -g -fno-omit-frame-pointer
            -fsanitize=address -fsanitize=undefined)

    target_link_options(Paths PRIVATE
            -rdynamic -fsanitize=address -fsanitize=undefined)
endif ()
