cmake_minimum_required(VERSION 3.23)
project(serialPro)

set(CMAKE_CXX_STANDARD 14)

include_directories(include)

add_executable(serialPro src/main.cpp)
