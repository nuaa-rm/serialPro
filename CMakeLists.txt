cmake_minimum_required(VERSION 3.3)
project(serialPro)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_BUILD_TYPE Debug)

include_directories(include msg_serializer thirdparty/serialib)

add_subdirectory(thirdparty/serialib)

add_executable(example_msg_serialize example/msg_serializer_test.cpp)

add_executable(example_serial_test example/robot_serial_test.cpp)
target_link_libraries(example_serial_test serialib)

add_executable(example_referee_serial_test example/referee_serial_test.cpp)
target_link_libraries(example_referee_serial_test serialib)
