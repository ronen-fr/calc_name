cmake_minimum_required(VERSION 3.20)
project(ritoa)

set(CMAKE_CXX_STANDARD 20)


find_package(benchmark REQUIRED)
find_package(fmt REQUIRED)

add_executable(ritoa main.cpp)

target_link_libraries(ritoa benchmark::benchmark fmt::fmt)
