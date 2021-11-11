cmake_minimum_required(VERSION 3.21)
project(libpaths_tests)

set(CMAKE_CXX_STANDARD 23)

add_subdirectory(googletest)

add_executable(${PROJECT_NAME}
        test_test.cpp
        test_maths.cpp
        )
target_include_directories(${PROJECT_NAME} PRIVATE googletest/googletest/include)
target_link_libraries(${PROJECT_NAME} libpaths gtest gtest_main)

add_executable(paths_maths_distrib_test
        distribTest.cpp
        )
target_include_directories(paths_maths_distrib_test PRIVATE googletest/googletest/include)
target_link_libraries(paths_maths_distrib_test libpaths)
