cmake_minimum_required(VERSION 3.10)
project(RayTracingInOneWeekend)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -O3 -march=native -fopenmp")

add_executable(RayTracingInOneWeekend main.cpp)

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(RayTracingInOneWeekend PUBLIC OpenMP::OpenMP_CXX)
endif()