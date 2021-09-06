cmake_minimum_required(VERSION 3.21)
project(paths_gfx)

add_library(${PROJECT_NAME} SHARED
        src/camera/body.cpp
        src/camera/camera.cpp
        src/image.cpp
        src/scene.cpp
        )
target_include_directories(${PROJECT_NAME} PUBLIC ./include)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
target_link_libraries(${PROJECT_NAME} paths_maths paths_utils omp)
target_compile_options(${PROJECT_NAME} PUBLIC -fopenmp)