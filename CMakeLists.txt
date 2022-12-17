cmake_minimum_required(VERSION 3.23)
project(serialPro)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_BUILD_TYPE Debug)

include_directories(include msg_serializer thirdparty/serialib)

set(BUILD_TESTS off)
add_subdirectory(thirdparty/serialib)

add_library(serialPro SHARED src/serialPro.cpp)

add_executable(example_msg_serialize example/msg_serializer_test.cpp)
target_link_libraries(example_msg_serialize serialPro serialib)
