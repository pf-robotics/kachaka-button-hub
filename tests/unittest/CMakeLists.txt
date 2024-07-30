cmake_minimum_required(VERSION 3.20)

project(baku_button_unittest)

set(CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wall -Wextra -Wno-long-long")

add_library(mock_libs SHARED mock_libs/Arduino.cpp)
target_include_directories(mock_libs PUBLIC mock_libs)

enable_testing()
find_package(GTest REQUIRED)
include(GoogleTest)

add_executable(test_robot_version tests/test_robot_version.cpp
                                  ../../button_hub/robot_version.cpp)
target_link_libraries(test_robot_version mock_libs GTest::GTest GTest::Main)
target_include_directories(test_robot_version PRIVATE ../../button_hub)

gtest_discover_tests(test_robot_version)
