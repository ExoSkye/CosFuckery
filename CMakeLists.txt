cmake_minimum_required(VERSION 3.25)
project(CosFuckery CXX)

set(CMAKE_CXX_STANDARD 17)

add_executable(CosFuckery main.cpp)
target_link_libraries(CosFuckery PUBLIC pthread)
